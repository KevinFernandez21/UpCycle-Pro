from fastapi import APIRouter, HTTPException, Depends
from fastapi.responses import JSONResponse
from pydantic import BaseModel, Field
from typing import Optional, Dict, List, Any
import requests
import boto3
import json
import logging
from datetime import datetime, timedelta
import yaml
import os

router = APIRouter()

# Configurar logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Modelos Pydantic
class MaterialVolumeData(BaseModel):
    material_type: str = Field(..., description="Tipo de material: glass, plastic, metal")
    volume: float = Field(..., description="Volumen del material en litros")
    weight: Optional[float] = Field(None, description="Peso del material en gramos")
    fragment_number: Optional[int] = Field(1, description="Número de fragmento del día (1-4)")
    quality_score: Optional[float] = Field(1.0, description="Puntuación de calidad (0-1)")
    processing_time: Optional[float] = Field(None, description="Tiempo de procesamiento en segundos")
    device_id: Optional[str] = Field(None, description="ID del dispositivo que envía los datos")
    metadata: Optional[Dict[str, Any]] = Field(None, description="Metadatos adicionales")

class PredictionRequest(BaseModel):
    days_back: Optional[int] = Field(30, description="Días históricos para entrenar")
    prediction_days: Optional[int] = Field(7, description="Días a predecir")

class AWSConfig:
    def __init__(self):
        self.config_path = "../../configs/aws.yaml"
        self.load_config()
    
    def load_config(self):
        try:
            if os.path.exists(self.config_path):
                with open(self.config_path, 'r') as file:
                    config = yaml.safe_load(file)
                    self.aws_endpoint = config.get('rnn_endpoint', '')
                    self.aws_region = config.get('region', 'us-east-1')
                    self.api_key = config.get('clave_api', '')
            else:
                # Configuración por defecto o variables de entorno
                self.aws_endpoint = os.environ.get('AWS_RNN_ENDPOINT', '')
                self.aws_region = os.environ.get('AWS_REGION', 'us-east-1')
                self.api_key = os.environ.get('AWS_API_KEY', '')
        except Exception as e:
            logger.error(f"Error cargando configuración AWS: {str(e)}")
            self.aws_endpoint = ''
            self.aws_region = 'us-east-1'
            self.api_key = ''

# Instancia global de configuración
aws_config = AWSConfig()

@router.post("/ingest-volume", response_model=Dict[str, Any])
async def ingest_material_volume(data: MaterialVolumeData):
    """
    Enviar datos de volumen de material al sistema AWS
    """
    try:
        # Preparar datos para AWS
        payload = {
            "action": "ingest",
            "material_type": data.material_type,
            "volume": data.volume,
            "weight": data.weight,
            "fragment_number": data.fragment_number,
            "quality_score": data.quality_score,
            "processing_time": data.processing_time,
            "timestamp": datetime.now().isoformat(),
            "device_id": data.device_id,
            "metadata": data.metadata
        }
        
        # Si hay endpoint AWS configurado, enviar a AWS
        if aws_config.aws_endpoint:
            headers = {
                'Content-Type': 'application/json',
                'x-api-key': aws_config.api_key
            } if aws_config.api_key else {'Content-Type': 'application/json'}
            
            response = requests.post(
                f"{aws_config.aws_endpoint}/ingest",
                json=payload,
                headers=headers,
                timeout=30
            )
            
            if response.status_code == 200:
                aws_result = response.json()
                logger.info(f"Datos enviados a AWS exitosamente: {aws_result}")
                return {
                    "status": "success",
                    "message": "Datos procesados y enviados a AWS",
                    "aws_response": aws_result,
                    "local_timestamp": datetime.now().isoformat()
                }
            else:
                logger.error(f"Error enviando a AWS: {response.status_code} - {response.text}")
                # Continuar con almacenamiento local como respaldo
        
        # Almacenamiento local como respaldo o principal
        local_result = {
            "status": "success",
            "message": "Datos procesados localmente",
            "data": payload,
            "local_timestamp": datetime.now().isoformat()
        }
        
        # Aquí podrías guardar en base de datos local si fuera necesario
        logger.info(f"Datos procesados localmente: {data.material_type} - {data.volume}L")
        
        return local_result
        
    except requests.exceptions.RequestException as e:
        logger.error(f"Error de conexión con AWS: {str(e)}")
        raise HTTPException(status_code=503, detail=f"Error de conexión con AWS: {str(e)}")
    except Exception as e:
        logger.error(f"Error procesando datos de volumen: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Error interno: {str(e)}")

@router.post("/predict-volume", response_model=Dict[str, Any])
async def predict_material_volume(request: PredictionRequest):
    """
    Obtener predicciones de volumen de material usando RNN en AWS
    """
    try:
        if not aws_config.aws_endpoint:
            raise HTTPException(
                status_code=503, 
                detail="Endpoint AWS no configurado. Configure aws.yaml con rnn_endpoint."
            )
        
        # Preparar payload para AWS Lambda
        payload = {
            "days_back": request.days_back,
            "prediction_days": request.prediction_days
        }
        
        headers = {
            'Content-Type': 'application/json',
            'x-api-key': aws_config.api_key
        } if aws_config.api_key else {'Content-Type': 'application/json'}
        
        # Llamar a AWS Lambda
        response = requests.post(
            f"{aws_config.aws_endpoint}/predict",
            json=payload,
            headers=headers,
            timeout=120  # Timeout más largo para predicciones
        )
        
        if response.status_code == 200:
            predictions = response.json()
            logger.info(f"Predicciones obtenidas exitosamente: {len(predictions.get('predictions', []))} días")
            
            # Agregar información adicional
            result = {
                "status": "success",
                "predictions": predictions,
                "request_info": {
                    "days_back": request.days_back,
                    "prediction_days": request.prediction_days,
                    "requested_at": datetime.now().isoformat()
                }
            }
            
            return result
        else:
            logger.error(f"Error en predicción AWS: {response.status_code} - {response.text}")
            raise HTTPException(
                status_code=response.status_code,
                detail=f"Error en predicción AWS: {response.text}"
            )
        
    except requests.exceptions.RequestException as e:
        logger.error(f"Error de conexión con AWS para predicción: {str(e)}")
        raise HTTPException(status_code=503, detail=f"Error de conexión con AWS: {str(e)}")
    except Exception as e:
        logger.error(f"Error obteniendo predicciones: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Error interno: {str(e)}")

@router.get("/daily-summary", response_model=Dict[str, Any])
async def get_daily_summary(date: Optional[str] = None):
    """
    Obtener resumen diario de volúmenes de material
    """
    try:
        if not aws_config.aws_endpoint:
            return {
                "status": "warning",
                "message": "Endpoint AWS no configurado. Mostrando datos de ejemplo.",
                "data": {
                    "date": date or datetime.now().strftime('%Y%m%d'),
                    "total_volume": 0,
                    "total_weight": 0,
                    "total_fragments": 0,
                    "materials": {
                        "glass": {"volume": 0, "weight": 0, "fragments": 0},
                        "plastic": {"volume": 0, "weight": 0, "fragments": 0},
                        "metal": {"volume": 0, "weight": 0, "fragments": 0}
                    }
                }
            }
        
        # Preparar payload
        payload = {
            "action": "summary",
            "date": date
        }
        
        headers = {
            'Content-Type': 'application/json',
            'x-api-key': aws_config.api_key
        } if aws_config.api_key else {'Content-Type': 'application/json'}
        
        # Llamar a AWS
        response = requests.post(
            f"{aws_config.aws_endpoint}/ingest",
            json=payload,
            headers=headers,
            timeout=30
        )
        
        if response.status_code == 200:
            summary = response.json()
            logger.info(f"Resumen diario obtenido: {summary.get('date', 'N/A')}")
            return {
                "status": "success",
                "summary": summary,
                "requested_at": datetime.now().isoformat()
            }
        else:
            logger.error(f"Error obteniendo resumen: {response.status_code} - {response.text}")
            raise HTTPException(
                status_code=response.status_code,
                detail=f"Error obteniendo resumen: {response.text}"
            )
        
    except requests.exceptions.RequestException as e:
        logger.error(f"Error de conexión con AWS para resumen: {str(e)}")
        raise HTTPException(status_code=503, detail=f"Error de conexión con AWS: {str(e)}")
    except Exception as e:
        logger.error(f"Error obteniendo resumen diario: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Error interno: {str(e)}")

@router.get("/config-status")
async def get_config_status():
    """
    Verificar estado de configuración AWS
    """
    return {
        "aws_endpoint_configured": bool(aws_config.aws_endpoint),
        "aws_region": aws_config.aws_region,
        "api_key_configured": bool(aws_config.api_key),
        "config_file_exists": os.path.exists(aws_config.config_path),
        "timestamp": datetime.now().isoformat()
    }

@router.post("/test-connection")
async def test_aws_connection():
    """
    Probar conexión con servicios AWS
    """
    try:
        if not aws_config.aws_endpoint:
            raise HTTPException(
                status_code=400,
                detail="Endpoint AWS no configurado"
            )
        
        # Hacer una llamada de prueba
        headers = {
            'Content-Type': 'application/json',
            'x-api-key': aws_config.api_key
        } if aws_config.api_key else {'Content-Type': 'application/json'}
        
        test_payload = {
            "action": "test",
            "timestamp": datetime.now().isoformat()
        }
        
        response = requests.post(
            f"{aws_config.aws_endpoint}/ingest",
            json=test_payload,
            headers=headers,
            timeout=10
        )
        
        return {
            "status": "success" if response.status_code == 200 else "error",
            "status_code": response.status_code,
            "response_time": response.elapsed.total_seconds(),
            "endpoint": aws_config.aws_endpoint,
            "timestamp": datetime.now().isoformat()
        }
        
    except requests.exceptions.RequestException as e:
        return {
            "status": "error",
            "error": str(e),
            "endpoint": aws_config.aws_endpoint,
            "timestamp": datetime.now().isoformat()
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error en prueba de conexión: {str(e)}")