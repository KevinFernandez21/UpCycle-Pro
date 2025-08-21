import json
import boto3
import logging
from datetime import datetime
from typing import Dict, Any
import os

# Configurar logging
logger = logging.getLogger()
logger.setLevel(logging.INFO)

class DataIngestionService:
    def __init__(self):
        self.dynamodb = boto3.resource('dynamodb')
        self.table_name = os.environ.get('DYNAMODB_TABLE', 'material-volume-data')
        self.table = self.dynamodb.Table(self.table_name)
        
    def process_material_data(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """Procesar datos de material desde microcontroladores"""
        try:
            # Validar datos requeridos
            required_fields = ['material_type', 'volume', 'timestamp']
            for field in required_fields:
                if field not in data:
                    raise ValueError(f"Campo requerido faltante: {field}")
            
            # Generar ID único para el material
            material_id = f"{data['material_type']}_{datetime.now().strftime('%Y%m%d')}"
            
            # Preparar item para DynamoDB
            item = {
                'material_id': material_id,
                'timestamp': data['timestamp'],
                'material_type': data['material_type'],  # glass, plastic, metal
                'volume': float(data['volume']),
                'weight': float(data.get('weight', 0)),
                'fragment_number': int(data.get('fragment_number', 1)),
                'quality_score': float(data.get('quality_score', 1.0)),
                'processing_time': float(data.get('processing_time', 0)),
                'created_at': datetime.now().isoformat(),
                'source': data.get('source', 'microcontroller'),
                'device_id': data.get('device_id', 'unknown')
            }
            
            # Agregar metadatos adicionales si están disponibles
            if 'metadata' in data:
                item['metadata'] = data['metadata']
            
            return item
            
        except Exception as e:
            logger.error(f"Error procesando datos: {str(e)}")
            raise
    
    def save_to_dynamodb(self, item: Dict[str, Any]) -> None:
        """Guardar datos en DynamoDB"""
        try:
            response = self.table.put_item(Item=item)
            logger.info(f"Datos guardados exitosamente: {item['material_id']}")
            return response
            
        except Exception as e:
            logger.error(f"Error guardando en DynamoDB: {str(e)}")
            raise
    
    def get_daily_summary(self, date: str = None) -> Dict[str, Any]:
        """Obtener resumen diario de volúmenes por tipo de material"""
        try:
            if not date:
                date = datetime.now().strftime('%Y%m%d')
            
            # Buscar todos los registros del día
            response = self.table.scan(
                FilterExpression=boto3.dynamodb.conditions.Attr('material_id').contains(date)
            )
            
            items = response['Items']
            
            # Calcular resumen por tipo de material
            summary = {
                'date': date,
                'total_volume': 0,
                'total_weight': 0,
                'total_fragments': 0,
                'materials': {
                    'glass': {'volume': 0, 'weight': 0, 'fragments': 0},
                    'plastic': {'volume': 0, 'weight': 0, 'fragments': 0},
                    'metal': {'volume': 0, 'weight': 0, 'fragments': 0}
                }
            }
            
            for item in items:
                material_type = item['material_type']
                volume = float(item['volume'])
                weight = float(item.get('weight', 0))
                
                summary['total_volume'] += volume
                summary['total_weight'] += weight
                summary['total_fragments'] += 1
                
                if material_type in summary['materials']:
                    summary['materials'][material_type]['volume'] += volume
                    summary['materials'][material_type]['weight'] += weight
                    summary['materials'][material_type]['fragments'] += 1
            
            return summary
            
        except Exception as e:
            logger.error(f"Error obteniendo resumen diario: {str(e)}")
            raise

def lambda_handler(event, context):
    """Handler principal para ingesta de datos"""
    try:
        # Inicializar servicio
        ingestion_service = DataIngestionService()
        
        # Obtener datos del evento
        if 'body' in event:
            body = json.loads(event['body']) if isinstance(event['body'], str) else event['body']
        else:
            body = event
        
        # Determinar acción
        action = body.get('action', 'ingest')
        
        if action == 'ingest':
            # Procesar datos de material
            processed_data = ingestion_service.process_material_data(body)
            
            # Guardar en DynamoDB
            ingestion_service.save_to_dynamodb(processed_data)
            
            return {
                'statusCode': 200,
                'headers': {
                    'Content-Type': 'application/json',
                    'Access-Control-Allow-Origin': '*'
                },
                'body': json.dumps({
                    'message': 'Datos procesados exitosamente',
                    'material_id': processed_data['material_id'],
                    'timestamp': processed_data['timestamp']
                })
            }
        
        elif action == 'summary':
            # Obtener resumen diario
            date = body.get('date')
            summary = ingestion_service.get_daily_summary(date)
            
            return {
                'statusCode': 200,
                'headers': {
                    'Content-Type': 'application/json',
                    'Access-Control-Allow-Origin': '*'
                },
                'body': json.dumps(summary, default=str)
            }
        
        else:
            return {
                'statusCode': 400,
                'body': json.dumps({
                    'error': f'Acción no válida: {action}',
                    'valid_actions': ['ingest', 'summary']
                })
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

# Función para procesar streams de DynamoDB
def stream_processor(event, context):
    """Procesar streams de DynamoDB para triggers automáticos"""
    try:
        for record in event['Records']:
            if record['eventName'] in ['INSERT', 'MODIFY']:
                # Procesar nuevos datos o modificaciones
                logger.info(f"Procesando evento: {record['eventName']}")
                
                # Aquí puedes agregar lógica adicional como:
                # - Validaciones en tiempo real
                # - Alertas por volúmenes anómalos
                # - Actualización de métricas en tiempo real
                
        return {
            'statusCode': 200,
            'body': json.dumps({'message': 'Stream procesado exitosamente'})
        }
        
    except Exception as e:
        logger.error(f"Error procesando stream: {str(e)}")
        return {
            'statusCode': 500,
            'body': json.dumps({'error': str(e)})
        }