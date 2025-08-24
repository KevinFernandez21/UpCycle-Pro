# UpCycle Pro - Sistema Integrado de Clasificación de Materiales

## 🚀 Nueva Arquitectura Mejorada

Este sistema ha sido completamente rediseñado para proporcionar una integración perfecta entre:
- **Frontend Flutter** - Interfaz móvil con stream en tiempo real
- **Backend FastAPI** - API robusta con WebSockets y comunicación ESP32
- **Microcontroladores ESP32** - Hardware inteligente con doble comunicación (MQTT + HTTP API)
- **Sistema CNN** - Clasificación de materiales con IA

## 📋 Características Principales

### ✨ Frontend Flutter Mejorado
- **Stream en tiempo real** desde ESP32-CAM
- **Interfaz optimizada** para Raspberry Pi 5
- **Control completo** de cámara (flash, calidad, reinicio)
- **Clasificación automática** con resultados detallados
- **Animaciones fluidas** y UX mejorada

### 🔧 Backend FastAPI Avanzado
- **Descubrimiento automático** de dispositivos ESP32
- **WebSockets** para comunicación en tiempo real
- **API REST completa** con documentación automática
- **Gestión de estado** distribuido y confiable
- **Integración CNN** con TensorFlow

### 🤖 ESP32 con Doble Protocolo
- **MQTT + Node-RED** para dashboards
- **HTTP API** para comunicación directa con backend
- **Auto-registro** en sistema backend
- **Comando queue** para operaciones asíncronas
- **Monitoreo en tiempo real** de sensores

## 🏗️ Arquitectura del Sistema

```
┌─────────────────┐    WebSocket    ┌──────────────────┐    HTTP/API    ┌─────────────────┐
│                 │ ◄─────────────► │                  │ ◄────────────► │                 │
│  Flutter App    │     REST API    │   FastAPI        │   MQTT/HTTP    │   ESP32 Device  │
│  (Raspberry Pi) │ ◄─────────────► │   Backend        │ ◄────────────► │   (Control)     │
│                 │                 │                  │                │                 │
└─────────────────┘                 └──────────────────┘                └─────────────────┘
                                            │                                      │
                                            │                                      │
                                            ▼                                      ▼
                                    ┌──────────────────┐                ┌─────────────────┐
                                    │                  │                │                 │
                                    │  CNN Model       │                │  ESP32-CAM      │
                                    │  Classification  │                │  (Vision)       │
                                    │                  │                │                 │
                                    └──────────────────┘                └─────────────────┘
```

## 🛠️ Instalación y Configuración

### 1. Backend FastAPI

```bash
# Navegar al directorio del backend
cd backend

# Instalar dependencias
pip install -r requirements.txt

# Configurar variables de entorno (opcional)
cp .env.example .env  # y editarlo según necesidades

# Iniciar servidor de desarrollo
python start_server.py --dev

# O iniciar servidor de producción
python start_server.py --prod --host 0.0.0.0 --port 8000
```

### 2. Frontend Flutter

```bash
# Navegar al directorio del frontend
cd frontend

# Instalar dependencias
flutter pub get

# Configurar IP del backend en lib/services/config_service.dart
# Cambiar apiBaseUrl a la IP de tu servidor backend

# Ejecutar en Raspberry Pi o dispositivo
flutter run -d linux  # Para Raspberry Pi
flutter run -d android  # Para Android
```

### 3. ESP32 Microcontrolador

1. **Abrir Arduino IDE** y cargar `microcontrolador/NodeRED_Integration/UpCyclePRO_NodeRED.ino`

2. **Instalar librerías necesarias:**
   - WiFi
   - ArduinoJson
   - PubSubClient
   - WebSocketsServer

3. **Configurar red:**
   ```cpp
   const char* ssid = "TU_RED_WIFI";
   const char* password = "TU_PASSWORD";
   const char* api_server = "192.168.1.100";  // IP del backend
   ```

4. **Cargar código** al ESP32

## 🧪 Pruebas del Sistema

### Prueba 1: Conectividad Backend

```bash
# Verificar que el backend está funcionando
curl http://localhost:8000/

# Ver documentación API
# Abrir navegador en: http://localhost:8000/docs

# Probar descubrimiento de dispositivos
curl -X POST http://localhost:8000/system/discover_devices

# Ver métricas del sistema
curl http://localhost:8000/system/metrics
```

### Prueba 2: WebSocket (usando wscat)

```bash
# Instalar wscat si no está instalado
npm install -g wscat

# Conectar a WebSocket principal
wscat -c ws://localhost:8000/ws/flutter_app

# Suscribirse a eventos de sensores
{"type": "subscribe", "event_type": "sensor_data"}

# Suscribirse a resultados de clasificación
{"type": "subscribe", "event_type": "classification_results"}
```

### Prueba 3: ESP32 Integration

```bash
# Registrar dispositivo ESP32 manualmente
curl -X POST "http://localhost:8000/api/esp32-cam/register" \
  -F "device_id=esp32_test_001" \
  -F "ip_address=192.168.1.50"

# Enviar comando a ESP32
curl -X POST "http://localhost:8000/api/esp32-control/command/esp32_test_001" \
  -H "Content-Type: application/json" \
  -d '{"command": "classify", "parameters": {"material": "plastic"}}'

# Ver datos de sensores
curl http://localhost:8000/api/esp32-control/sensors/esp32_test_001
```

### Prueba 4: Clasificación Completa

```bash
# Capturar imagen de ESP32-CAM y clasificar
curl -X POST http://localhost:8000/system/capture_and_classify

# O subir imagen manualmente
curl -X POST "http://localhost:8000/predict" \
  -F "file=@test_image.jpg"
```

## 📡 Endpoints API Principales

### Sistema General
- `GET /` - Info de la API
- `GET /health` - Estado de salud
- `GET /system/metrics` - Métricas completas
- `POST /system/discover_devices` - Descubrir ESP32s
- `POST /system/capture_and_classify` - Captura + clasificación

### ESP32 Integration
- `POST /api/esp32-cam/register` - Registrar ESP32-CAM
- `GET /api/esp32-cam/status` - Estado de dispositivos CAM
- `GET /api/esp32-cam/capture/{device_id}` - Capturar imagen
- `POST /api/esp32-cam/control/{device_id}` - Controlar cámara
- `POST /api/esp32-control/sensors` - Actualizar sensores
- `POST /api/esp32-control/command/{device_id}` - Enviar comando

### WebSocket
- `ws://host:port/ws/flutter_app` - WebSocket principal
- `ws://host:port/ws/camera_stream` - Stream de cámara

### Clasificación
- `POST /predict` - Clasificar imagen
- `POST /classification/result` - Registrar resultado
- `GET /classification/history` - Historial

## 🔧 Configuración Avanzada

### Variables de Entorno (.env)

```env
# API Configuration
API_HOST=0.0.0.0
API_PORT=8000
API_RELOAD=true

# ESP32 Configuration
ESP32_DISCOVERY_ENABLED=true
ESP32_TIMEOUT=10
ESP32_RETRY_ATTEMPTS=3

# Network Configuration
NETWORK_BASE_IP=192.168.1.
NETWORK_SCAN_RANGE=254

# CNN Model Configuration
MODEL_PATH=../ai_client/CNN
MODEL_CONFIDENCE_THRESHOLD=0.7

# WebSocket Configuration
WS_MAX_CONNECTIONS=100
ENABLE_WEBSOCKETS=true

# Feature Flags
ENABLE_MQTT=true
ENABLE_AUTO_CLASSIFICATION=true
ENABLE_DEVICE_REGISTRATION=true
```

### Personalización de Servos

En `backend/api/config.py`:

```python
SERVO_POSITIONS = {
    "glass": 45,     # Vidrio a 45 grados
    "plastic": 90,   # Plástico a 90 grados
    "metal": 135     # Metal a 135 grados
}
```

## 🐛 Solución de Problemas

### Problema: ESP32 no se conecta al backend

1. Verificar que las IPs están correctas en el código ESP32
2. Asegurar que el backend está corriendo en el puerto correcto
3. Verificar conectividad de red:
   ```bash
   ping 192.168.1.100  # IP del backend
   ```

### Problema: Frontend no recibe stream de cámara

1. Verificar WebSocket connection en logs del backend
2. Asegurar que ESP32-CAM está registrado:
   ```bash
   curl http://localhost:8000/api/esp32-cam/status
   ```

### Problema: Clasificación falla

1. Verificar que el modelo CNN está cargado:
   ```bash
   curl http://localhost:8000/model_info
   ```
2. Verificar disponibilidad de modelos:
   ```bash
   curl http://localhost:8000/available_models
   ```

### Problema: WebSocket se desconecta

1. Verificar logs del servidor para errores
2. Implementar reconexión automática en Flutter:
   ```dart
   // En el servicio WebSocket
   void _scheduleReconnect() {
     Timer(Duration(seconds: 5), () => _connect());
   }
   ```

## 📊 Monitoreo y Logs

### Logs del Backend

```bash
# Ver logs en tiempo real
tail -f backend/logs/app.log

# Logs específicos de ESP32
grep "ESP32" backend/logs/app.log

# Logs de WebSocket
grep "WebSocket" backend/logs/app.log
```

### Métricas en Tiempo Real

Acceder a `http://localhost:8000/system/metrics` para ver:
- Estado de dispositivos ESP32
- Estadísticas de WebSocket
- Rendimiento del modelo CNN
- Historial de clasificaciones

## 🚀 Despliegue en Producción

### Usando Docker (Recomendado)

```dockerfile
# Dockerfile para el backend
FROM python:3.9

WORKDIR /app
COPY requirements.txt .
RUN pip install -r requirements.txt

COPY . .
CMD ["python", "start_server.py", "--prod", "--workers", "4"]
```

```bash
# Construir y ejecutar
docker build -t upcycle-backend .
docker run -p 8000:8000 upcycle-backend
```

### Usando Systemd (Linux)

```ini
# /etc/systemd/system/upcycle-backend.service
[Unit]
Description=UpCycle Pro Backend
After=network.target

[Service]
Type=forking
User=pi
WorkingDirectory=/home/pi/UpCycle-Pro/backend
ExecStart=/usr/bin/python3 start_server.py --prod
Restart=always

[Install]
WantedBy=multi-user.target
```

```bash
# Activar servicio
sudo systemctl enable upcycle-backend
sudo systemctl start upcycle-backend
```

## 📈 Próximas Mejoras

1. **Dashboard Web** - Interfaz web para monitoreo
2. **Base de Datos** - Persistencia de datos históricos
3. **Autenticación** - Sistema de usuarios y permisos
4. **Escalabilidad** - Soporte para múltiples líneas de producción
5. **Analytics** - Reportes y estadísticas avanzadas
6. **Mobile App** - Aplicación nativa para smartphones

## 🤝 Contribuir

1. Fork el repositorio
2. Crear feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit cambios (`git commit -m 'Add AmazingFeature'`)
4. Push al branch (`git push origin feature/AmazingFeature`)
5. Abrir Pull Request

## 📞 Soporte

Para soporte técnico o preguntas:
- Revisar logs del sistema
- Verificar configuración de red
- Consultar documentación API en `/docs`

---

**UpCycle Pro v2.0** - Sistema de Clasificación Automática de Materiales con IA