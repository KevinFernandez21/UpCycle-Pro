from fastapi import FastAPI, File, UploadFile, HTTPException
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
from routes.microcontroller import router as microcontroller_router

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

# Incluir rutas del microcontrolador
app.include_router(microcontroller_router, prefix="/microcontroller", tags=["microcontroller"])

# Variables globales
model = None
class_names = ['glass', 'metal', 'plastic']
IMG_SIZE = (224, 224)

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

if __name__ == "__main__":
    uvicorn.run(
        "main:app",   # Cambia esto si tu archivo tiene otro nombre
        host="192.168.0.108",
        port=8000,
        reload=True,
        log_level="info"
    )