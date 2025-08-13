from fastapi import APIRouter, HTTPException
from pydantic import BaseModel
from typing import Dict, Any, Optional
import logging
from datetime import datetime

router = APIRouter()
logger = logging.getLogger(__name__)

class SensorData(BaseModel):
    pir_sensor: bool
    weight: float
    timestamp: Optional[str] = None

class MotorCommand(BaseModel):
    conveyor_active: bool
    servo_position: int  # 0-180 degrees
    duration: Optional[int] = None  # seconds

class SystemStatus(BaseModel):
    system_active: bool
    last_classification: Optional[str] = None
    motor_status: Optional[str] = None
    servo_position: Optional[int] = None

# Estado global del sistema (en producción usar una base de datos)
system_state = {
    "active": False,
    "last_classification": None,
    "motor_active": False,
    "servo_position": 90,
    "sensor_data": {
        "pir_sensor": False,
        "weight": 0.0,
        "last_update": None
    }
}

@router.post("/sensor/update")
async def update_sensor_data(data: SensorData):
    """Endpoint para que el microcontrolador envíe datos de sensores"""
    try:
        system_state["sensor_data"]["pir_sensor"] = data.pir_sensor
        system_state["sensor_data"]["weight"] = data.weight
        system_state["sensor_data"]["last_update"] = data.timestamp or datetime.now().isoformat()
        
        logger.info(f"Datos de sensores actualizados: PIR={data.pir_sensor}, Peso={data.weight}kg")
        
        # Lógica de activación automática basada en sensores
        if data.pir_sensor and data.weight > 0.1:  # Objeto detectado con peso significativo
            system_state["active"] = True
            logger.info("Sistema activado automáticamente por detección de objeto")
        
        return {
            "status": "success",
            "message": "Datos de sensores actualizados",
            "system_activated": system_state["active"]
        }
    except Exception as e:
        logger.error(f"Error actualizando datos de sensores: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Error interno: {str(e)}")

@router.post("/motor/control")
async def control_motor(command: MotorCommand):
    """Endpoint para controlar motores del sistema"""
    try:
        system_state["motor_active"] = command.conveyor_active
        system_state["servo_position"] = command.servo_position
        
        logger.info(f"Comando de motor: Conveyor={command.conveyor_active}, Servo={command.servo_position}°")
        
        # Aquí se enviaría el comando real al microcontrolador
        # Por ahora solo actualizamos el estado
        
        return {
            "status": "success",
            "message": "Comando de motor enviado",
            "conveyor_active": command.conveyor_active,
            "servo_position": command.servo_position
        }
    except Exception as e:
        logger.error(f"Error controlando motor: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Error interno: {str(e)}")

@router.get("/status")
async def get_system_status():
    """Obtener estado actual del sistema"""
    return SystemStatus(
        system_active=system_state["active"],
        last_classification=system_state["last_classification"],
        motor_status="active" if system_state["motor_active"] else "inactive",
        servo_position=system_state["servo_position"]
    )

@router.post("/classification/result")
async def update_classification_result(result: Dict[str, Any]):
    """Actualizar resultado de clasificación y activar sistema de separación"""
    try:
        predicted_class = result.get("predicted_class", "unknown")
        confidence = result.get("confidence", 0.0)
        
        system_state["last_classification"] = predicted_class
        
        # Lógica de separación basada en el material detectado
        servo_positions = {
            "glass": 45,      # Posición para vidrio
            "plastic": 90,    # Posición para plástico
            "metal": 135      # Posición para metal
        }
        
        target_position = servo_positions.get(predicted_class, 90)
        
        # Activar sistema de separación si la confianza es alta
        if confidence > 0.7:
            system_state["servo_position"] = target_position
            system_state["motor_active"] = True
            
            logger.info(f"Sistema de separación activado: {predicted_class} -> servo {target_position}°")
            
            # Aquí se enviaría el comando al microcontrolador para mover el servo
            # y activar el motor transportador
            
            return {
                "status": "success",
                "message": f"Sistema activado para separar {predicted_class}",
                "servo_position": target_position,
                "motor_activated": True,
                "confidence": confidence
            }
        else:
            logger.warning(f"Confianza insuficiente ({confidence:.2f}) para activar separación")
            return {
                "status": "low_confidence",
                "message": "Confianza insuficiente para separación automática",
                "confidence": confidence,
                "threshold": 0.7
            }
            
    except Exception as e:
        logger.error(f"Error procesando resultado de clasificación: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Error interno: {str(e)}")

@router.post("/system/activate")
async def activate_system():
    """Activar manualmente el sistema"""
    system_state["active"] = True
    logger.info("Sistema activado manualmente")
    return {"status": "success", "message": "Sistema activado"}

@router.post("/system/deactivate")
async def deactivate_system():
    """Desactivar el sistema"""
    system_state["active"] = False
    system_state["motor_active"] = False
    system_state["servo_position"] = 90  # Posición neutral
    logger.info("Sistema desactivado")
    return {"status": "success", "message": "Sistema desactivado"}

@router.get("/sensor/data")
async def get_sensor_data():
    """Obtener datos actuales de los sensores"""
    return system_state["sensor_data"]