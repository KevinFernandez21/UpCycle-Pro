# UpCycle Pro - Sistema Simplificado API Backend

## 🎯 Arquitectura Simplificada

Sistema UpCyclePRO completamente rediseñado con comunicación **directa API Backend** sin Node-RED ni MQTT:

- **Frontend Flutter** ↔ **Backend FastAPI** ↔ **ESP32 Microcontrolador**
- Comunicación HTTP/API única y directa
- WebSockets para tiempo real
- Sin dependencias externas (MQTT/Node-RED)

## 📋 Funcionalidades Principales

### 🔧 ESP32 Microcontrolador
- ✅ **Comunicación HTTP directa** con backend
- ✅ **Auto-registro** automático al iniciar
- ✅ **Servidor web local** para control directo
- ✅ **Envío periódico** de datos de sensores (cada 5 seg)
- ✅ **Procesamiento de comandos** del backend
- ✅ **Confirmación de resultados** de clasificación
- ✅ **Estado de conexión** en tiempo real

### 🖥️ Backend FastAPI
- ✅ **API REST completa** para ESP32 y Flutter
- ✅ **WebSockets** para comunicación en tiempo real
- ✅ **Sistema de cola de comandos** para ESP32
- ✅ **Descubrimiento automático** de dispositivos
- ✅ **Registro y gestión** de dispositivos ESP32
- ✅ **Integración CNN** para clasificación

### 📱 Frontend Flutter
- ✅ **Interfaz simplificada** y optimizada
- ✅ **Stream en tiempo real** de cámara y datos
- ✅ **Control total** del sistema de clasificación
- ✅ **WebSocket bidireccional** para actualizaciones

## 🏗️ Arquitectura del Sistema

```
┌─────────────────┐    HTTP/WebSocket   ┌──────────────────┐    HTTP API    ┌─────────────────┐
│                 │ ◄─────────────────► │                  │ ◄────────────► │                 │
│  Flutter App    │                     │   FastAPI        │                │   ESP32 Device  │
│  (Frontend)     │                     │   Backend        │                │   (Control)     │
│                 │                     │                  │                │                 │
└─────────────────┘                     └──────────────────┘                └─────────────────┘
                                                │                                      │
                                                │                                      │
                                                ▼                                      ▼
                                        ┌──────────────────┐                ┌─────────────────┐
                                        │                  │                │                 │
                                        │  CNN Model       │                │  Sensores       │
                                        │  Classification  │                │  Servos         │
                                        │                  │                │  PIR            │
                                        └──────────────────┘                └─────────────────┘
```

## 🛠️ Instalación Rápida

### 1. Backend FastAPI

```bash
# Navegar al directorio del backend
cd backend

# Instalar dependencias
pip install -r requirements.txt

# Iniciar servidor
python start_server.py --dev
```

**El backend estará disponible en:** `http://localhost:8000`

### 2. ESP32 Microcontrolador

1. **Cargar el código Arduino:**
   - Abrir `microcontrolador/NodeRED_Integration/UpCyclePRO_NodeRED/UpCyclePRO_NodeRED.ino`
   - Cambiar configuración WiFi y backend:
   ```cpp
   const char* ssid = "TU_RED_WIFI";
   const char* password = "TU_PASSWORD";
   const char* api_server = "192.168.1.100";  // IP del backend
   ```

2. **Librerías necesarias:**
   - WiFi (incluida con ESP32)
   - HTTPClient (incluida con ESP32)
   - ArduinoJson
   - WebServer (incluida con ESP32)

3. **Cargar al ESP32**

### 3. Frontend Flutter

```bash
cd frontend
flutter pub get

# Editar lib/services/config_service.dart
# Cambiar apiBaseUrl a la IP de tu backend

flutter run
```

## 🧪 Probar el Sistema

### Prueba Automática Completa

```bash
cd backend
python test_esp32_connection.py
```

Este script probará automáticamente:
- ✅ Estado del backend
- ✅ Conectividad del ESP32
- ✅ Registro de dispositivo
- ✅ Envío de datos de sensores
- ✅ Comandos de clasificación
- ✅ Métricas del sistema

### Pruebas Manuales

#### 1. Verificar Backend
```bash
curl http://localhost:8000/health
curl http://localhost:8000/system/metrics
```

#### 2. Verificar ESP32
```bash
# Cambiar IP por la de tu ESP32
curl http://192.168.1.50/status
curl http://192.168.1.50/sensors
```

#### 3. Probar Clasificación
```bash
# Enviar comando via backend
curl -X POST "http://localhost:8000/api/esp32-control/command/esp32_main_001" \
  -H "Content-Type: application/json" \
  -d '{"command": "classify", "parameters": {"material": "plastic"}}'

# O directamente al ESP32
curl -X POST "http://192.168.1.50/classify" \
  -H "Content-Type: application/json" \
  -d '{"material": "glass"}'
```

## 📡 API Endpoints

### Backend FastAPI

| Endpoint | Método | Descripción |
|----------|--------|-------------|
| `/health` | GET | Estado del backend |
| `/system/metrics` | GET | Métricas completas |
| `/api/esp32-cam/register` | POST | Registrar ESP32 |
| `/api/esp32-control/sensors` | POST | Recibir datos sensores |
| `/api/esp32-control/command/{device_id}` | POST | Enviar comando |
| `/api/classification/result` | POST | Resultado clasificación |
| `/ws/flutter_app` | WebSocket | Conexión Flutter |

### ESP32 Local

| Endpoint | Método | Descripción |
|----------|--------|-------------|
| `/status` | GET | Estado completo |
| `/sensors` | GET | Lectura sensores |
| `/classify` | POST | Comando clasificación |
| `/start` | POST | Iniciar sistema |
| `/stop` | POST | Detener sistema |

## 🔄 Flujo de Operación

### 1. Inicio del Sistema
1. **ESP32** se conecta a WiFi
2. **ESP32** se auto-registra con el backend
3. **Backend** agrega ESP32 a lista de dispositivos
4. **ESP32** comienza envío periódico de sensores

### 2. Proceso de Clasificación
1. **Frontend** solicita clasificación
2. **Backend** encola comando para ESP32
3. **ESP32** recibe comando en próximo envío de sensores
4. **ESP32** ejecuta clasificación y mueve servo
5. **ESP32** envía resultado al backend
6. **Backend** notifica a Frontend via WebSocket

### 3. Monitoreo Continuo
- **ESP32** → **Backend**: Datos cada 5 segundos
- **Backend** → **Frontend**: WebSocket en tiempo real
- **Backend**: Ping de salud cada minuto

## 📊 Monitoreo y Logs

### Ver Estado del Sistema
```bash
# Métricas en tiempo real
curl http://localhost:8000/system/metrics

# Estado específico del ESP32
curl http://192.168.1.50/status
```

### Logs del Backend
```bash
# En la terminal donde corre el backend
# Los logs aparecen en tiempo real mostrando:
# - Registro de dispositivos
# - Datos de sensores recibidos  
# - Comandos enviados
# - Resultados de clasificación
```

### Logs del ESP32
```bash
# En el monitor serie del Arduino IDE:
# - Conexión WiFi
# - Registro con backend
# - Comandos recibidos
# - Estado de sensores
# - Ejecución de servos
```

## ⚙️ Configuración

### Variables Backend (.env)
```env
API_HOST=0.0.0.0
API_PORT=8000
ESP32_DISCOVERY_ENABLED=true
ESP32_TIMEOUT=10
SENSOR_INTERVAL=5000
BACKEND_PING_INTERVAL=60000
```

### Variables ESP32 (código)
```cpp
const char* ssid = "TU_RED_WIFI";
const char* password = "TU_PASSWORD"; 
const char* api_server = "192.168.1.100";
const int api_port = 8000;
String device_id = "esp32_main_001";
```

## 🛡️ Solución de Problemas

### ESP32 no se conecta al Backend

1. **Verificar conectividad:**
   ```bash
   ping 192.168.1.50  # IP del ESP32
   curl http://192.168.1.50/status
   ```

2. **Verificar logs ESP32:**
   - Abrir monitor serie Arduino IDE
   - Buscar mensajes de error de conexión

3. **Verificar configuración:**
   - IP del backend en código ESP32
   - Backend corriendo en puerto correcto

### Frontend no recibe actualizaciones

1. **Verificar WebSocket:**
   ```bash
   # Probar WebSocket manualmente
   wscat -c ws://localhost:8000/ws/flutter_app
   ```

2. **Verificar logs backend:**
   - Buscar conexiones WebSocket
   - Verificar mensajes de broadcast

### Clasificación no funciona

1. **Verificar modelo CNN:**
   ```bash
   curl http://localhost:8000/model_info
   ```

2. **Probar comando directo:**
   ```bash
   curl -X POST "http://192.168.1.50/classify" \
     -H "Content-Type: application/json" \
     -d '{"material": "plastic"}'
   ```

3. **Verificar cola de comandos:**
   ```bash
   curl http://localhost:8000/system/metrics
   # Ver "pending_commands" en respuesta
   ```

## 🚀 Ventajas del Sistema Simplificado

### ✅ Pros
- **Menos complejidad**: Sin MQTT ni Node-RED
- **Comunicación directa**: Menor latencia
- **Fácil debug**: Solo HTTP requests
- **Auto-recuperación**: ESP32 se reconecta automáticamente
- **Estado consistente**: Una sola fuente de verdad (backend)

### ⚠️ Consideraciones
- **Dependiente de WiFi**: Sistema requiere red estable
- **Polling periódico**: ESP32 consulta cada 5 segundos
- **Sin dashboard Node-RED**: Solo interfaz Flutter

## 📈 Próximas Mejoras

1. **Autenticación**: Sistema de tokens JWT
2. **Base de datos**: Persistencia de datos históricos  
3. **Dashboard web**: Interfaz web complementaria
4. **Alertas**: Notificaciones de errores
5. **Logs estructurados**: Mejor sistema de logging
6. **Configuración remota**: Cambiar parámetros sin recompilar

---

**UpCycle Pro v2.1** - Sistema Simplificado de Clasificación con API Directa

*Documentación actualizada: Agosto 2024*