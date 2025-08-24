from fastapi import FastAPI, File, UploadFile, HTTPException, WebSocket, WebSocketDisconnect
from fastapi.responses import JSONResponse
from fastapi.middleware.cors import CORSMiddleware
import tensorflow as tf
import numpy as np
from PIL import Image
import io
import logging
from typing import Dict, List
import uvicorn
from datetime import datetime
import os
import json
import asyncio

# Import routers and services
from routes.microcontroller import router as microcontroller_router
from routes.esp32_integration import router as esp32_router
from services.system_service import SystemService
from websocket_manager import websocket_manager

# Configurar logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Configuración de la aplicación
app = FastAPI(
    title="Material Classification API",
    description="API para clasificación de materiales (vidrio, plástico, metal) usando CNN",
    version="1.0.0",
    docs_url="/docs",
    redoc_url="/redoc"
)

# Configurar CORS para permitir conexiones desde Raspberry Pi
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # En producción, especifica IPs específicas
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Incluir rutas
app.include_router(microcontroller_router, prefix="/microcontroller", tags=["microcontroller"])
app.include_router(esp32_router, prefix="/api", tags=["esp32"])

# Variables globales
model = None
class_names = ['glass', 'metal', 'plastic']
IMG_SIZE = (224, 224)
system_service = SystemService()

def create_dummy_model():
    """Crear un modelo dummy para pruebas cuando no se encuentra el modelo real"""
    from tensorflow.keras.models import Sequential
    from tensorflow.keras.layers import Dense, GlobalAveragePooling2D
    from tensorflow.keras.applications import MobileNetV2

    base_model = MobileNetV2(weights='imagenet', include_top=False, input_shape=(224, 224, 3))
    base_model.trainable = False

    model = Sequential([
        base_model,
        GlobalAveragePooling2D(),
        Dense(128, activation='relu'),
        Dense(3, activation='softmax')
    ])

    model.compile(
        optimizer='adam',
        loss='categorical_crossentropy',
        metrics=['accuracy']
    )

    return model

# Cargar el modelo al iniciar la aplicación
@app.on_event("startup")
async def load_model():
    global model
    try:
        # Buscar modelos en la carpeta CNN
        cnn_folder = "../ai_client/CNN"
        model_files = ["modelo_corregido_materiales.keras", "best_model.keras", "best_model.h5"]
        
        model_loaded = False
        for model_file in model_files:
            model_path = os.path.join(cnn_folder, model_file)
            if os.path.exists(model_path):
                try:
                    loaded_model = tf.keras.models.load_model(model_path)
                    logger.info(f"Modelo cargado exitosamente desde {model_path}")
                    logger.info(f"Métodos disponibles en el modelo: {dir(loaded_model)}")
                    # Validar si el modelo tiene método predict
                    if hasattr(loaded_model, "predict"):
                        model = loaded_model
                        model_loaded = True
                        break
                    else:
                        logger.error(f"El modelo en {model_path} no tiene el método 'predict'.")
                except Exception as e:
                    logger.error(f"Error cargando modelo {model_path}: {str(e)}")
                    continue
        
        if not model_loaded:
            logger.error(f"No se pudo cargar ningún modelo desde {cnn_folder}")
            logger.warning("Creando modelo dummy para pruebas...")
            model = create_dummy_model()
    except Exception as e:
        logger.error(f"Error al cargar el modelo: {str(e)}")
        logger.warning("Creando modelo dummy para pruebas...")
        model = create_dummy_model()

def preprocess_image(image: Image.Image) -> np.ndarray:
    """Preprocesar la imagen para el modelo"""
    try:
        if image.mode != 'RGB':
            image = image.convert('RGB')
        image = image.resize(IMG_SIZE)
        img_array = np.array(image)
        img_array = img_array.astype('float32') / 255.0
        img_array = np.expand_dims(img_array, axis=0)
        return img_array
    except Exception as e:
        logger.error(f"Error en preprocesamiento: {str(e)}")
        raise HTTPException(status_code=400, detail=f"Error al procesar imagen: {str(e)}")

@app.get("/")
async def root():
    return {
        "message": "Material Classification API",
        "version": "1.0.0",
        "status": "running",
        "model_loaded": model is not None,
        "classes": class_names,
        "docs": "/docs",
        "timestamp": datetime.now().isoformat()
    }

@app.get("/health")
async def health_check():
    return {
        "status": "healthy",
        "model_status": "loaded" if model is not None else "not_loaded",
        "timestamp": datetime.now().isoformat()
    }

@app.post("/predict", response_model=Dict)
async def predict_image(file: UploadFile = File(...)):
    if model is None:
        raise HTTPException(status_code=503, detail="Modelo no cargado")

    if not file.content_type.startswith('image/'):
        raise HTTPException(status_code=400, detail="El archivo debe ser una imagen")

    try:
        image_data = await file.read()
        image = Image.open(io.BytesIO(image_data))
        processed_image = preprocess_image(image)
        predictions = model.predict(processed_image)
        predicted_class_idx = int(np.argmax(predictions[0]))
        predicted_class = class_names[predicted_class_idx]
        confidence = float(predictions[0][predicted_class_idx])

        result = {
            "predicted_class": predicted_class,
            "confidence": confidence,
            "all_probabilities": {
                class_names[i]: float(predictions[0][i])
                for i in range(len(class_names))
            },
            "timestamp": datetime.now().isoformat(),
            "image_info": {
                "filename": file.filename,
                "content_type": file.content_type,
                "size": len(image_data)
            }
        }

        logger.info(f"Predicción: {predicted_class} con confianza {confidence:.4f}")
        
        # Comunicar resultado al sistema de microcontrolador
        try:
            from routes.microcontroller import system_state
            system_state["last_classification"] = predicted_class
            
            # Activar sistema de separación si la confianza es alta
            if confidence > 0.7:
                servo_positions = {
                    "glass": 45,
                    "plastic": 90,
                    "metal": 135
                }
                target_position = servo_positions.get(predicted_class, 90)
                system_state["servo_position"] = target_position
                system_state["motor_active"] = True
                logger.info(f"Sistema de separación activado automáticamente: {predicted_class} -> {target_position}°")
        except Exception as e:
            logger.warning(f"Error comunicando con sistema de microcontrolador: {e}")
            
        return result
    except Exception as e:
        logger.error(f"Error en predicción: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Error al procesar imagen: {str(e)}")

@app.post("/predict_batch", response_model=List[Dict])
async def predict_batch_images(files: List[UploadFile] = File(...)):
    if model is None:
        raise HTTPException(status_code=503, detail="Modelo no cargado")

    if len(files) > 10:
        raise HTTPException(status_code=400, detail="Máximo 10 imágenes por lote")

    results = []
    for i, file in enumerate(files):
        try:
            image_data = await file.read()
            image = Image.open(io.BytesIO(image_data))
            processed_image = preprocess_image(image)
            predictions = model.predict(processed_image)
            predicted_class_idx = int(np.argmax(predictions[0]))
            predicted_class = class_names[predicted_class_idx]
            confidence = float(predictions[0][predicted_class_idx])

            results.append({
                "index": i,
                "filename": file.filename,
                "predicted_class": predicted_class,
                "confidence": confidence,
                "all_probabilities": {
                    class_names[j]: float(predictions[0][j])
                    for j in range(len(class_names))
                }
            })
        except Exception as e:
            results.append({
                "index": i,
                "filename": file.filename,
                "error": str(e),
                "predicted_class": None,
                "confidence": 0.0
            })
    return results

@app.get("/model_info")
async def get_model_info():
    if model is None:
        raise HTTPException(status_code=503, detail="Modelo no cargado")
    try:
        return {
            "model_type": str(type(model)),
            "input_shape": getattr(model, "input_shape", "N/A"),
            "output_shape": getattr(model, "output_shape", "N/A"),
            "classes": class_names,
            "num_classes": len(class_names),
            "trainable_params": model.count_params() if hasattr(model, "count_params") else "N/A"
        }
    except Exception as e:
        return {"error": str(e)}

@app.post("/load_model")
async def load_cnn_model(model_name: str = "modelo_corregido_materiales.keras"):
    """Endpoint para cargar manualmente un modelo CNN específico"""
    global model
    try:
        cnn_folder = "../ai_client/CNN"
        model_path = os.path.join(cnn_folder, model_name)
        
        if not os.path.exists(model_path):
            available_models = []
            if os.path.exists(cnn_folder):
                available_models = [f for f in os.listdir(cnn_folder) if f.endswith(('.h5', '.keras'))]
            raise HTTPException(
                status_code=404, 
                detail=f"Modelo '{model_name}' no encontrado. Modelos disponibles: {available_models}"
            )
        
        loaded_model = tf.keras.models.load_model(model_path)
        
        if not hasattr(loaded_model, "predict"):
            raise HTTPException(
                status_code=400, 
                detail=f"El modelo '{model_name}' no tiene el método 'predict'"
            )
        
        model = loaded_model
        logger.info(f"Modelo '{model_name}' cargado exitosamente desde {model_path}")
        
        return {
            "status": "success",
            "message": f"Modelo '{model_name}' cargado exitosamente",
            "model_path": model_path,
            "model_type": str(type(model)),
            "input_shape": getattr(model, "input_shape", "N/A"),
            "output_shape": getattr(model, "output_shape", "N/A"),
            "timestamp": datetime.now().isoformat()
        }
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error cargando modelo '{model_name}': {str(e)}")
        raise HTTPException(status_code=500, detail=f"Error al cargar modelo: {str(e)}")

@app.get("/available_models")
async def get_available_models():
    """Obtener lista de modelos disponibles en la carpeta CNN"""
    try:
        cnn_folder = "../ai_client/CNN"
        if not os.path.exists(cnn_folder):
            return {"models": [], "message": "Carpeta CNN no encontrada"}
        
        model_files = [f for f in os.listdir(cnn_folder) if f.endswith(('.h5', '.keras'))]
        
        models_info = []
        for model_file in model_files:
            model_path = os.path.join(cnn_folder, model_file)
            file_size = os.path.getsize(model_path)
            models_info.append({
                "filename": model_file,
                "path": model_path,
                "size_bytes": file_size,
                "size_mb": round(file_size / (1024 * 1024), 2)
            })
        
        return {
            "models": models_info,
            "count": len(models_info),
            "current_model_loaded": model is not None
        }
        
    except Exception as e:
        logger.error(f"Error obteniendo modelos disponibles: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Error al obtener modelos: {str(e)}")

# WebSocket Endpoints
@app.websocket("/ws/{client_type}")
async def websocket_endpoint(websocket: WebSocket, client_type: str):
    """WebSocket principal para comunicación en tiempo real"""
    await websocket_manager.connect(websocket, client_type)
    try:
        while True:
            # Recibir mensajes del cliente
            message_text = await websocket.receive_text()
            try:
                message = json.loads(message_text)
                await websocket_manager.handle_client_message(websocket, message)
            except json.JSONDecodeError:
                logger.error(f"Invalid JSON received from WebSocket: {message_text}")
                
    except WebSocketDisconnect:
        await websocket_manager.disconnect(websocket, client_type)
    except Exception as e:
        logger.error(f"WebSocket error: {e}")
        await websocket_manager.disconnect(websocket, client_type)

@app.websocket("/ws/camera_stream")
async def camera_stream_websocket(websocket: WebSocket):
    """WebSocket específico para stream de cámara"""
    await websocket.accept()
    
    try:
        # Suscribir automáticamente a camera_stream
        await websocket_manager.subscribe_to_event(websocket, "camera_stream")
        
        while True:
            # Mantener conexión viva
            await asyncio.sleep(1)
            
    except WebSocketDisconnect:
        await websocket_manager.unsubscribe_from_event(websocket, "camera_stream")
    except Exception as e:
        logger.error(f"Camera stream WebSocket error: {e}")

# Endpoints mejorados que integran con el sistema ESP32
@app.post("/system/capture_and_classify")
async def capture_and_classify_from_esp32():
    """Capturar imagen de ESP32-CAM y clasificar"""
    if model is None:
        raise HTTPException(status_code=503, detail="Modelo no cargado")
    
    try:
        # Capturar imagen desde ESP32-CAM
        image_bytes = await system_service.capture_image_from_esp32()
        if image_bytes is None:
            raise HTTPException(status_code=503, detail="No se pudo capturar imagen de ESP32-CAM")
        
        # Clasificar imagen
        result = await system_service.classify_image(image_bytes, model)
        if result is None:
            raise HTTPException(status_code=500, detail="Error en clasificación")
        
        # Enviar comando al ESP32-CONTROL si la confianza es alta
        if result["confidence"] > 0.7:
            classification_success = await system_service.send_classification_command(
                material=result["predicted_class"],
                servo_position=result["servo_position"]
            )
            
            result["system_action"] = {
                "servo_command_sent": classification_success,
                "servo_position": result["servo_position"],
                "material": result["predicted_class"]
            }
        else:
            result["system_action"] = {
                "servo_command_sent": False,
                "reason": "Low confidence",
                "threshold": 0.7
            }
        
        # Broadcast resultado via WebSocket
        await websocket_manager.broadcast_classification_result(result)
        
        logger.info(f"Complete classification: {result['predicted_class']} ({result['confidence']:.3f})")
        
        return result
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error in capture_and_classify: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/system/metrics")
async def get_system_metrics():
    """Obtener métricas completas del sistema"""
    try:
        # Obtener métricas del sistema ESP32
        esp32_metrics = await system_service.get_system_metrics()
        
        # Agregar métricas de WebSocket
        websocket_stats = websocket_manager.get_connection_stats()
        
        # Agregar información del modelo
        model_info = {
            "loaded": model is not None,
            "classes": class_names,
            "input_shape": getattr(model, "input_shape", None) if model else None
        }
        
        return {
            "timestamp": datetime.now().isoformat(),
            "system_health": "healthy" if esp32_metrics["connectivity"]["online_devices"] > 0 else "degraded",
            "esp32_devices": esp32_metrics,
            "websocket_connections": websocket_stats,
            "ml_model": model_info,
            "api_version": "2.0.0"
        }
        
    except Exception as e:
        logger.error(f"Error getting system metrics: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/system/discover_devices")
async def discover_esp32_devices():
    """Descubrir dispositivos ESP32 en la red"""
    try:
        discovered = await system_service.discover_esp32_devices()
        
        # Broadcast estado de dispositivos via WebSocket
        await websocket_manager.broadcast_device_status({
            "action": "discovery_complete",
            "discovered_devices": discovered,
            "timestamp": datetime.now().isoformat()
        })
        
        return {
            "status": "success",
            "discovered_devices": discovered,
            "total_devices": len(discovered["esp32_cam"]) + len(discovered["esp32_control"]),
            "timestamp": datetime.now().isoformat()
        }
        
    except Exception as e:
        logger.error(f"Error discovering devices: {e}")
        raise HTTPException(status_code=500, detail=str(e))

# Background task para actualización periódica del sistema
@app.on_event("startup")
async def startup_periodic_tasks():
    """Iniciar tareas periódicas del sistema"""
    
    async def periodic_device_discovery():
        """Descubrir dispositivos periódicamente"""
        while True:
            try:
                await asyncio.sleep(300)  # Cada 5 minutos
                discovered = await system_service.discover_esp32_devices()
                
                # Broadcast si hay cambios
                await websocket_manager.broadcast_device_status({
                    "action": "periodic_discovery",
                    "discovered_devices": discovered,
                    "timestamp": datetime.now().isoformat()
                })
                
            except Exception as e:
                logger.error(f"Error in periodic device discovery: {e}")
    
    async def periodic_system_status():
        """Enviar estado del sistema periódicamente"""
        while True:
            try:
                await asyncio.sleep(30)  # Cada 30 segundos
                
                # Obtener métricas
                metrics = await system_service.get_system_metrics()
                
                # Broadcast estado via WebSocket
                await websocket_manager.broadcast_system_status({
                    "system_metrics": metrics,
                    "model_loaded": model is not None,
                    "api_version": "2.0.0",
                    "timestamp": datetime.now().isoformat()
                })
                
            except Exception as e:
                logger.error(f"Error in periodic system status: {e}")
    
    # Iniciar tareas en background
    asyncio.create_task(periodic_device_discovery())
    asyncio.create_task(periodic_system_status())
    
    # Descubrimiento inicial
    try:
        await system_service.discover_esp32_devices()
        logger.info("Initial ESP32 device discovery completed")
    except Exception as e:
        logger.error(f"Error in initial device discovery: {e}")

if __name__ == "__main__":
    uvicorn.run(
        "main:app",
        host="0.0.0.0",  # Escuchar en todas las interfaces
        port=8000,
        reload=True,
        log_level="info"
    )