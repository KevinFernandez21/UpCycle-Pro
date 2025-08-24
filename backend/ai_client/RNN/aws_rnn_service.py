import json
import boto3
import torch
import numpy as np
import pandas as pd
from datetime import datetime, timedelta
from chronos import ChronosPipeline
import logging
from typing import Dict, List, Any
import os

# Configurar logging
logger = logging.getLogger()
logger.setLevel(logging.INFO)

class MaterialVolumePredictor:
    def __init__(self):
        self.pipeline = None
        self.dynamodb = boto3.resource('dynamodb')
        self.table_name = os.environ.get('DYNAMODB_TABLE', 'material-volume-data')
        self.table = self.dynamodb.Table(self.table_name)
        self.s3_client = boto3.client('s3')
        self.model_bucket = os.environ.get('MODEL_BUCKET', 'upcycle-pro-models')
        
    def load_model(self):
        """Cargar el modelo Chronos desde S3 o Hugging Face"""
        try:
            if not self.pipeline:
                logger.info("Cargando modelo Chronos...")
                self.pipeline = ChronosPipeline.from_pretrained(
                    "amazon/chronos-t5-large",
                    device_map="cpu",  # Lambda usa CPU
                    torch_dtype=torch.float32,
                )
                logger.info("Modelo cargado exitosamente")
        except Exception as e:
            logger.error(f"Error cargando modelo: {str(e)}")
            raise
    
    def get_daily_data(self, days_back: int = 30) -> pd.DataFrame:
        """Obtener datos diarios de volumen de material de DynamoDB"""
        try:
            # Calcular fechas
            end_date = datetime.now()
            start_date = end_date - timedelta(days=days_back)
            
            # Query DynamoDB para obtener datos históricos
            response = self.table.scan(
                FilterExpression=boto3.dynamodb.conditions.Attr('timestamp').between(
                    start_date.isoformat(),
                    end_date.isoformat()
                )
            )
            
            # Convertir a DataFrame
            items = response['Items']
            df = pd.DataFrame(items)
            
            if df.empty:
                logger.warning("No se encontraron datos históricos")
                return pd.DataFrame()
            
            # Procesar datos
            df['timestamp'] = pd.to_datetime(df['timestamp'])
            df['volume'] = pd.to_numeric(df['volume'])
            
            # Agrupar por día y calcular volumen total
            daily_data = df.groupby(df['timestamp'].dt.date)['volume'].sum().reset_index()
            daily_data.columns = ['date', 'total_volume']
            daily_data = daily_data.sort_values('date')
            
            logger.info(f"Datos obtenidos: {len(daily_data)} días")
            return daily_data
            
        except Exception as e:
            logger.error(f"Error obteniendo datos: {str(e)}")
            raise
    
    def predict_volume(self, historical_data: pd.DataFrame, prediction_days: int = 7) -> Dict[str, Any]:
        """Realizar predicción de volumen para los próximos días"""
        try:
            if historical_data.empty:
                raise ValueError("No hay datos históricos suficientes")
            
            # Preparar contexto para el modelo
            context = torch.tensor(historical_data['total_volume'].values, dtype=torch.float32)
            
            # Realizar predicción
            forecast = self.pipeline.predict(context, prediction_days)
            
            # Procesar resultados
            forecast_array = forecast[0].numpy()
            low, median, high = np.quantile(forecast_array, [0.1, 0.5, 0.9], axis=0)
            
            # Crear fechas futuras
            last_date = historical_data['date'].iloc[-1]
            future_dates = [last_date + timedelta(days=i+1) for i in range(prediction_days)]
            
            predictions = []
            for i, date in enumerate(future_dates):
                predictions.append({
                    'date': date.isoformat(),
                    'predicted_volume': float(median[i]),
                    'lower_bound': float(low[i]),
                    'upper_bound': float(high[i]),
                    'confidence_interval': '80%'
                })
            
            result = {
                'predictions': predictions,
                'model_info': {
                    'model_type': 'chronos-t5-large',
                    'prediction_days': prediction_days,
                    'historical_days': len(historical_data),
                    'timestamp': datetime.now().isoformat()
                },
                'summary': {
                    'avg_predicted_volume': float(np.mean(median)),
                    'total_predicted_volume': float(np.sum(median)),
                    'trend': 'increasing' if median[-1] > median[0] else 'decreasing'
                }
            }
            
            return result
            
        except Exception as e:
            logger.error(f"Error en predicción: {str(e)}")
            raise
    
    def save_predictions(self, predictions: Dict[str, Any]) -> None:
        """Guardar predicciones en DynamoDB"""
        try:
            prediction_table = self.dynamodb.Table('material-predictions')
            
            # Guardar cada predicción
            for pred in predictions['predictions']:
                item = {
                    'prediction_id': f"pred_{pred['date']}_{datetime.now().strftime('%Y%m%d_%H%M%S')}",
                    'prediction_date': pred['date'],
                    'predicted_volume': pred['predicted_volume'],
                    'lower_bound': pred['lower_bound'],
                    'upper_bound': pred['upper_bound'],
                    'created_at': datetime.now().isoformat(),
                    'model_version': 'chronos-t5-large',
                    'confidence_interval': pred['confidence_interval']
                }
                prediction_table.put_item(Item=item)
            
            # Guardar resumen
            summary_item = {
                'summary_id': f"summary_{datetime.now().strftime('%Y%m%d_%H%M%S')}",
                'created_at': datetime.now().isoformat(),
                'avg_predicted_volume': predictions['summary']['avg_predicted_volume'],
                'total_predicted_volume': predictions['summary']['total_predicted_volume'],
                'trend': predictions['summary']['trend'],
                'prediction_count': len(predictions['predictions'])
            }
            prediction_table.put_item(Item=summary_item)
            
            logger.info(f"Predicciones guardadas: {len(predictions['predictions'])} días")
            
        except Exception as e:
            logger.error(f"Error guardando predicciones: {str(e)}")
            raise

def lambda_handler(event, context):
    """Handler principal de AWS Lambda"""
    try:
        # Inicializar predictor
        predictor = MaterialVolumePredictor()
        predictor.load_model()
        
        # Obtener parámetros del evento
        days_back = event.get('days_back', 30)
        prediction_days = event.get('prediction_days', 7)
        
        # Obtener datos históricos
        historical_data = predictor.get_daily_data(days_back)
        
        if historical_data.empty:
            return {
                'statusCode': 400,
                'body': json.dumps({
                    'error': 'No hay datos históricos suficientes',
                    'message': f'Se necesitan al menos {days_back} días de datos'
                })
            }
        
        # Realizar predicción
        predictions = predictor.predict_volume(historical_data, prediction_days)
        
        # Guardar predicciones
        predictor.save_predictions(predictions)
        
        # Respuesta exitosa
        return {
            'statusCode': 200,
            'headers': {
                'Content-Type': 'application/json',
                'Access-Control-Allow-Origin': '*'
            },
            'body': json.dumps(predictions, default=str)
        }
        
    except Exception as e:
        logger.error(f"Error en lambda_handler: {str(e)}")
        return {
            'statusCode': 500,
            'body': json.dumps({
                'error': str(e),
                'message': 'Error interno del servidor'
            })
        }

# Función para ejecutar predicciones programadas
def scheduled_prediction_handler(event, context):
    """Handler para predicciones programadas diarias"""
    try:
        logger.info("Iniciando predicción programada diaria")
        
        # Usar el handler principal con parámetros por defecto
        prediction_event = {
            'days_back': 30,
            'prediction_days': 7
        }
        
        result = lambda_handler(prediction_event, context)
        
        if result['statusCode'] == 200:
            logger.info("Predicción programada completada exitosamente")
        else:
            logger.error(f"Error en predicción programada: {result['body']}")
        
        return result
        
    except Exception as e:
        logger.error(f"Error en predicción programada: {str(e)}")
        return {
            'statusCode': 500,
            'body': json.dumps({
                'error': str(e),
                'message': 'Error en predicción programada'
            })
        }