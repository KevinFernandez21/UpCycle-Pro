from fastapi import APIRouter, HTTPException, File, UploadFile, BackgroundTasks, Depends
from fastapi.responses import StreamingResponse, JSONResponse
from pydantic import BaseModel
from typing import Dict, List, Optional, Union
import aiohttp
import asyncio
import logging
from datetime import datetime, timedelta
import json
import io
from PIL import Image
import numpy as np
import base64

router = APIRouter()
logger = logging.getLogger(__name__)

class ESP32Status(BaseModel):
    device_id: str
    device_type: str  # "esp32-cam" or "esp32-control"
    ip_address: str
    status: str  # "online", "offline", "error"
    last_seen: datetime
    wifi_rssi: Optional[int] = None
    uptime: Optional[int] = None
    free_heap: Optional[int] = None

class ESP32CamControl(BaseModel):
    flash_enabled: Optional[bool] = None
    quality: Optional[int] = None  # 0-63
    resolution: Optional[str] = None
    action: Optional[str] = None  # "restart", "capture", "stream"

class ESP32SensorData(BaseModel):
    device_id: str
    timestamp: datetime
    pir1: Optional[bool] = None  # Vidrio
    pir2: Optional[bool] = None  # Plástico  
    pir3: Optional[bool] = None  # Metal
    weight1: Optional[float] = None  # Peso vidrio (kg)
    weight2: Optional[float] = None  # Peso plástico (kg)
    weight3: Optional[float] = None  # Peso metal (kg)
    conveyor_active: Optional[bool] = None
    servo_positions: Optional[Dict[str, int]] = None

class ESP32Classification(BaseModel):
    device_id: str
    material: str  # "glass", "plastic", "metal"
    confidence: float
    servo_position: int
    processing_time: Optional[float] = None
    timestamp: datetime

class ESP32Command(BaseModel):
    device_id: str
    command: str  # "classify", "stop", "start_conveyor", "move_servo"
    parameters: Optional[Dict] = None
    priority: Optional[int] = 1  # 1=high, 2=medium, 3=low

# Global state management
esp32_devices = {}  # device_id -> ESP32Status
sensor_history = []  # List of sensor readings
classification_history = []  # List of classifications
pending_commands = {}  # device_id -> List of commands

@router.get("/esp32-cam/status")
async def get_esp32_cam_status():
    """Obtener estado de todos los dispositivos ESP32-CAM"""
    try:
        cam_devices = {k: v for k, v in esp32_devices.items() if v.device_type == "esp32-cam"}
        return {
            "status": "success",
            "devices": cam_devices,
            "total_devices": len(cam_devices),
            "online_devices": sum(1 for d in cam_devices.values() if d.status == "online"),
            "timestamp": datetime.now()
        }
    except Exception as e:
        logger.error(f"Error getting ESP32-CAM status: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/esp32-cam/register")
async def register_esp32_device(device_id: str, ip_address: str, device_type: str = "esp32-control"):
    """Registrar un nuevo dispositivo ESP32 (CAM o CONTROL)"""
    try:
        # Verificar si el dispositivo está realmente online
        try:
            async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=5)) as session:
                async with session.get(f"http://{ip_address}/status") as response:
                    if response.status == 200:
                        device_info = await response.json()
                        
                        esp32_devices[device_id] = ESP32Status(
                            device_id=device_id,
                            device_type=device_type,
                            ip_address=ip_address,
                            status="online",
                            last_seen=datetime.now(),
                            wifi_rssi=device_info.get("wifi_rssi", -50),
                            uptime=device_info.get("uptime", 0),
                            free_heap=device_info.get("free_heap", 0)
                        )
                        
                        logger.info(f"ESP32 {device_type} {device_id} registered successfully at {ip_address}")
                        return {
                            "status": "success",
                            "message": f"ESP32 {device_type} {device_id} registered",
                            "device_info": esp32_devices[device_id]
                        }
        except Exception:
            # Dispositivo no responde, registrar como offline
            esp32_devices[device_id] = ESP32Status(
                device_id=device_id,
                device_type=device_type,
                ip_address=ip_address,
                status="offline",
                last_seen=datetime.now()
            )
            
            return {
                "status": "warning",
                "message": f"ESP32 {device_type} {device_id} registered but appears offline",
                "device_info": esp32_devices[device_id]
            }
            
    except Exception as e:
        logger.error(f"Error registering ESP32 device: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/esp32-cam/capture/{device_id}")
async def capture_image_from_device(device_id: str):
    """Capturar imagen de un ESP32-CAM específico"""
    try:
        if device_id not in esp32_devices:
            raise HTTPException(status_code=404, detail=f"Device {device_id} not found")
        
        device = esp32_devices[device_id]
        if device.status != "online":
            raise HTTPException(status_code=503, detail=f"Device {device_id} is offline")
        
        async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=10)) as session:
            async with session.get(f"http://{device.ip_address}/capture") as response:
                if response.status == 200:
                    image_data = await response.read()
                    
                    # Actualizar último visto
                    device.last_seen = datetime.now()
                    
                    return StreamingResponse(
                        io.BytesIO(image_data),
                        media_type="image/jpeg",
                        headers={"Content-Disposition": f"attachment; filename=esp32_{device_id}_capture.jpg"}
                    )
                else:
                    raise HTTPException(status_code=response.status, detail="Failed to capture image")
                    
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error capturing image from {device_id}: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/esp32-cam/control/{device_id}")
async def control_esp32_cam(device_id: str, control: ESP32CamControl):
    """Controlar configuraciones del ESP32-CAM"""
    try:
        if device_id not in esp32_devices:
            raise HTTPException(status_code=404, detail=f"Device {device_id} not found")
        
        device = esp32_devices[device_id]
        if device.status != "online":
            raise HTTPException(status_code=503, detail=f"Device {device_id} is offline")
        
        control_data = {}
        if control.flash_enabled is not None:
            control_data["flash"] = control.flash_enabled
        if control.quality is not None:
            control_data["quality"] = max(0, min(63, control.quality))
        if control.resolution is not None:
            control_data["resolution"] = control.resolution
        
        if control.action:
            if control.action == "restart":
                endpoint = "/restart"
                method = "POST"
            elif control.action == "capture":
                return await capture_image_from_device(device_id)
            else:
                raise HTTPException(status_code=400, detail=f"Unknown action: {control.action}")
        else:
            endpoint = "/control"
            method = "POST"
        
        async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=10)) as session:
            if method == "POST":
                async with session.post(
                    f"http://{device.ip_address}{endpoint}",
                    json=control_data if control_data else {}
                ) as response:
                    if response.status == 200:
                        device.last_seen = datetime.now()
                        return {
                            "status": "success",
                            "message": f"Control command sent to {device_id}",
                            "action": control.action or "settings_update",
                            "parameters": control_data
                        }
                    else:
                        raise HTTPException(status_code=response.status, detail="Device control failed")
                        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error controlling ESP32-CAM {device_id}: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/esp32-control/sensors")
async def update_sensor_data(sensor_data: ESP32SensorData):
    """Endpoint para que ESP32-CONTROL envíe datos de sensores"""
    try:
        # Registrar/actualizar dispositivo
        if sensor_data.device_id not in esp32_devices:
            esp32_devices[sensor_data.device_id] = ESP32Status(
                device_id=sensor_data.device_id,
                device_type="esp32-control",
                ip_address="auto-discovered",
                status="online",
                last_seen=datetime.now()
            )
        else:
            esp32_devices[sensor_data.device_id].last_seen = datetime.now()
            esp32_devices[sensor_data.device_id].status = "online"
        
        # Guardar datos de sensores
        sensor_history.append(sensor_data.dict())
        
        # Mantener solo los últimos 1000 registros
        if len(sensor_history) > 1000:
            sensor_history.pop(0)
        
        logger.info(f"Sensor data updated from {sensor_data.device_id}")
        
        # Retornar comandos pendientes para este dispositivo
        pending = pending_commands.get(sensor_data.device_id, [])
        if pending:
            # Limpiar comandos enviados
            pending_commands[sensor_data.device_id] = []
        
        return {
            "status": "success",
            "message": "Sensor data received",
            "pending_commands": pending,
            "device_registered": sensor_data.device_id in esp32_devices
        }
        
    except Exception as e:
        logger.error(f"Error processing sensor data: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/esp32-control/sensors/{device_id}")
async def get_sensor_data(device_id: str, limit: int = 50):
    """Obtener datos históricos de sensores"""
    try:
        device_data = [
            record for record in sensor_history 
            if record["device_id"] == device_id
        ]
        
        # Obtener los más recientes
        recent_data = device_data[-limit:] if device_data else []
        
        return {
            "status": "success",
            "device_id": device_id,
            "records": recent_data,
            "total_records": len(recent_data),
            "latest_timestamp": recent_data[-1]["timestamp"] if recent_data else None
        }
        
    except Exception as e:
        logger.error(f"Error getting sensor data for {device_id}: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/esp32-control/command/{device_id}")
async def send_command_to_device(device_id: str, command: ESP32Command):
    """Enviar comando a un dispositivo ESP32-CONTROL"""
    try:
        if device_id not in esp32_devices:
            raise HTTPException(status_code=404, detail=f"Device {device_id} not found")
        
        # Agregar comando a la cola
        if device_id not in pending_commands:
            pending_commands[device_id] = []
        
        command_dict = {
            "command": command.command,
            "parameters": command.parameters or {},
            "priority": command.priority or 1,
            "timestamp": datetime.now().isoformat(),
            "id": f"cmd_{len(pending_commands[device_id])}"
        }
        
        pending_commands[device_id].append(command_dict)
        
        # Ordenar por prioridad (1 = alta prioridad primero)
        pending_commands[device_id].sort(key=lambda x: x["priority"])
        
        logger.info(f"Command queued for {device_id}: {command.command}")
        
        return {
            "status": "success",
            "message": f"Command queued for {device_id}",
            "command": command_dict,
            "queue_size": len(pending_commands[device_id])
        }
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error queuing command for {device_id}: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/classification/result")
async def record_classification_result(classification: ESP32Classification):
    """Registrar resultado de clasificación"""
    try:
        # Guardar resultado
        classification_history.append(classification.dict())
        
        # Mantener solo los últimos 500 registros
        if len(classification_history) > 500:
            classification_history.pop(0)
        
        # Enviar comando para activar servo correspondiente
        servo_command = ESP32Command(
            device_id=classification.device_id,
            command="move_servo",
            parameters={
                "material": classification.material,
                "position": classification.servo_position,
                "duration": 2000  # 2 segundos
            },
            priority=1
        )
        
        await send_command_to_device(classification.device_id, servo_command)
        
        logger.info(f"Classification recorded: {classification.material} with {classification.confidence:.2f} confidence")
        
        return {
            "status": "success",
            "message": "Classification recorded and servo command queued",
            "classification": classification.dict()
        }
        
    except Exception as e:
        logger.error(f"Error recording classification: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/classification/history")
async def get_classification_history(limit: int = 100):
    """Obtener historial de clasificaciones"""
    try:
        recent_classifications = classification_history[-limit:] if classification_history else []
        
        # Estadísticas
        material_counts = {}
        total_confidence = 0
        
        for record in recent_classifications:
            material = record["material"]
            material_counts[material] = material_counts.get(material, 0) + 1
            total_confidence += record["confidence"]
        
        avg_confidence = total_confidence / len(recent_classifications) if recent_classifications else 0
        
        return {
            "status": "success",
            "classifications": recent_classifications,
            "total_records": len(recent_classifications),
            "statistics": {
                "material_counts": material_counts,
                "average_confidence": round(avg_confidence, 3)
            }
        }
        
    except Exception as e:
        logger.error(f"Error getting classification history: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/system/status")
async def get_system_status():
    """Obtener estado general del sistema"""
    try:
        online_devices = sum(1 for d in esp32_devices.values() if d.status == "online")
        total_devices = len(esp32_devices)
        
        # Últimos sensores
        latest_sensors = sensor_history[-1] if sensor_history else None
        
        # Última clasificación
        latest_classification = classification_history[-1] if classification_history else None
        
        return {
            "status": "success",
            "system_health": "healthy" if online_devices > 0 else "degraded",
            "devices": {
                "total": total_devices,
                "online": online_devices,
                "offline": total_devices - online_devices
            },
            "latest_sensor_data": latest_sensors,
            "latest_classification": latest_classification,
            "pending_commands": sum(len(cmds) for cmds in pending_commands.values()),
            "uptime_seconds": (datetime.now() - datetime(2024, 1, 1)).total_seconds(),
            "timestamp": datetime.now()
        }
        
    except Exception as e:
        logger.error(f"Error getting system status: {e}")
        raise HTTPException(status_code=500, detail=str(e))

# Funciones de limpieza automática
@router.on_event("startup")
async def cleanup_offline_devices():
    """Limpiar dispositivos offline periódicamente"""
    async def cleanup_task():
        while True:
            try:
                await asyncio.sleep(300)  # 5 minutos
                cutoff_time = datetime.now() - timedelta(minutes=10)
                
                offline_devices = []
                for device_id, device in esp32_devices.items():
                    if device.last_seen < cutoff_time:
                        device.status = "offline"
                        offline_devices.append(device_id)
                
                if offline_devices:
                    logger.info(f"Marked {len(offline_devices)} devices as offline")
                    
            except Exception as e:
                logger.error(f"Error in cleanup task: {e}")
    
    asyncio.create_task(cleanup_task())