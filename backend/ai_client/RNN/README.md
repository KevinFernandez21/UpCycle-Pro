# UpCycle Pro RNN - Predicción de Volúmenes de Material

Sistema de predicción de volúmenes de material reciclable utilizando RNN (Chronos) desplegado en AWS.

## 📋 Descripción

Este sistema utiliza el modelo Chronos de Amazon para predecir volúmenes futuros de materiales reciclables basándose en datos históricos diarios. Está diseñado para procesar 4 fragmentos de datos por día y generar predicciones automáticas.

## 🏗️ Arquitectura

### Componentes AWS:
- **AWS Lambda**: Funciones para predicción e ingesta de datos
- **DynamoDB**: Almacenamiento de datos históricos y predicciones
- **API Gateway**: Endpoints REST para integración
- **S3**: Almacenamiento de modelos y archivos
- **CloudWatch Events**: Predicciones automáticas diarias

### Componentes Locales:
- **FastAPI**: API local con endpoints para RNN
- **Routes**: Integración con servicios AWS

## 🚀 Instalación y Despliegue

### Prerequisitos
```bash
# Instalar Node.js y npm
# Instalar AWS CLI
pip install awscli

# Configurar credenciales AWS
aws configure
```

### Despliegue en AWS
```bash
cd backend/ai_client/RNN
chmod +x deploy.sh
./deploy.sh dev  # o 'prod' para producción
```

### Configuración Local
1. Actualizar `backend/configs/aws.yaml` con los endpoints generados
2. Instalar dependencias Python:
```bash
cd backend/api
pip install pydantic requests boto3 pyyaml
```

## 📊 Uso del Sistema

### 1. Envío de Datos de Volumen
```python
import requests

# Datos de ejemplo de un fragmento diario
data = {
    "material_type": "plastic",
    "volume": 2.5,
    "weight": 150.0,
    "fragment_number": 1,  # 1-4 fragmentos por día
    "quality_score": 0.85,
    "device_id": "sensor_01"
}

response = requests.post(
    "http://localhost:8000/rnn/ingest-volume",
    json=data
)
```

### 2. Obtener Predicciones
```python
prediction_request = {
    "days_back": 30,      # Días históricos para entrenar
    "prediction_days": 7  # Días a predecir
}

response = requests.post(
    "http://localhost:8000/rnn/predict-volume",
    json=prediction_request
)

predictions = response.json()
```

### 3. Resumen Diario
```python
# Resumen del día actual
response = requests.get("http://localhost:8000/rnn/daily-summary")

# Resumen de fecha específica
response = requests.get("http://localhost:8000/rnn/daily-summary?date=20241201")
```

## 🔧 Endpoints API

### Ingesta de Datos
- **POST** `/rnn/ingest-volume`
  - Envía datos de volumen al sistema AWS
  - Almacena en DynamoDB para predicciones futuras

### Predicciones
- **POST** `/rnn/predict-volume`
  - Genera predicciones usando modelo Chronos
  - Parámetros: `days_back`, `prediction_days`

### Monitoreo
- **GET** `/rnn/daily-summary`
  - Resumen de volúmenes por tipo de material
  - Opcional: parámetro `date`

- **GET** `/rnn/config-status`
  - Estado de configuración AWS

- **POST** `/rnn/test-connection`
  - Prueba conexión con servicios AWS

## 📈 Estructura de Datos

### Datos de Entrada (por fragmento)
```json
{
  "material_type": "glass|plastic|metal",
  "volume": 2.5,
  "weight": 150.0,
  "fragment_number": 1,
  "quality_score": 0.85,
  "processing_time": 12.3,
  "device_id": "sensor_01",
  "metadata": {}
}
```

### Predicciones de Salida
```json
{
  "predictions": [
    {
      "date": "2024-12-08",
      "predicted_volume": 45.2,
      "lower_bound": 38.1,
      "upper_bound": 52.3,
      "confidence_interval": "80%"
    }
  ],
  "summary": {
    "avg_predicted_volume": 42.5,
    "total_predicted_volume": 297.5,
    "trend": "increasing"
  }
}
```

## ⚙️ Configuración

### AWS Configuration (`backend/configs/aws.yaml`)
```yaml
rnn_endpoint: "https://your-api-gateway-url.amazonaws.com/prod"
clave_api: "your-api-key"
region: "us-east-1"

dynamodb:
  material_volume_table: "upcycle-pro-rnn-material-volume-prod"
  predictions_table: "upcycle-pro-rnn-predictions-prod"

s3:
  model_bucket: "upcycle-pro-models-prod"
```

### Variables de Entorno
```bash
export AWS_RNN_ENDPOINT="https://your-api-gateway-url.amazonaws.com/prod"
export AWS_API_KEY="your-api-key"
export AWS_REGION="us-east-1"
```

## 🔄 Automatización

### Predicciones Diarias Automáticas
El sistema está configurado para ejecutar predicciones automáticamente cada día usando CloudWatch Events.

### Procesamiento de Fragmentos
- El sistema recibe 4 fragmentos de datos por día
- Cada fragmento incluye volumen y metadatos del material
- Los datos se agregan diariamente para entrenar el modelo

## 🎯 Integración con Microcontroladores

### Desde el microcontrolador, envía datos así:
```python
# En tu código del microcontrolador
def send_volume_data(material_type, volume, weight=None):
    data = {
        "material_type": material_type,
        "volume": volume,
        "weight": weight,
        "fragment_number": get_current_fragment_number(),
        "device_id": "raspberry_pi_01",
        "timestamp": datetime.now().isoformat()
    }
    
    response = requests.post(
        "http://your-backend:8000/rnn/ingest-volume",
        json=data
    )
    return response.json()
```

## 🐛 Troubleshooting

### Problemas Comunes

1. **Error de conexión AWS**
   - Verificar credenciales: `aws sts get-caller-identity`
   - Verificar endpoint en `aws.yaml`

2. **Modelo no carga**
   - Verificar memoria Lambda (mínimo 3008 MB)
   - Verificar timeout Lambda (mínimo 900 segundos)

3. **Datos no se guardan**
   - Verificar permisos DynamoDB
   - Verificar formato de datos de entrada

### Logs
```bash
# Ver logs de Lambda
serverless logs -f predictVolume --stage dev

# Ver logs locales
tail -f backend/logs/app.log
```

## 📊 Monitoreo

### Métricas importantes:
- Volumen total diario por tipo de material
- Precisión de predicciones vs datos reales
- Tiempo de respuesta de predicciones
- Errores en ingesta de datos

### Dashboard recomendado:
- CloudWatch Dashboard para métricas AWS
- Grafana para visualización de predicciones
- Alertas por volúmenes anómalos

## 🔮 Próximas Mejoras

1. **Modelos Especializados**: Entrenar modelos específicos por tipo de material
2. **Predicciones Multivariate**: Incluir factores externos (clima, eventos)
3. **Alertas Inteligentes**: Notificaciones por patrones anómalos
4. **Optimización de Costos**: Auto-scaling basado en uso
5. **Validación en Tiempo Real**: Comparar predicciones vs datos reales

## 📞 Soporte

Para problemas o preguntas:
1. Verificar logs en CloudWatch
2. Probar endpoints con `/rnn/test-connection`
3. Revisar configuración en `/rnn/config-status`