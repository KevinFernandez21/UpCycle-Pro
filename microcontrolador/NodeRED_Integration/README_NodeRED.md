# 🚀 UpCyclePro - Integración Node-RED

## 📋 Descripción

Esta carpeta contiene las adaptaciones del sistema UpCyclePro para funcionar con Node-RED, incluyendo tanto el microcontrolador clasificador como la cámara ESP32-CAM.

## 📁 Archivos Incluidos

### 🎯 Código Arduino
- `UpCyclePRO_NodeRED.ino` - Microcontrolador clasificador adaptado
- `CameraWebServer_NodeRED.ino` - ESP32-CAM adaptado para Node-RED
- `flows_example.json` - Flows de ejemplo para Node-RED

### 📚 Librerías Necesarias

Para ambos proyectos Arduino:
```
ArduinoJson (6.x)
PubSubClient (MQTT)
WebSockets by Markus Sattler
```

Para el microcontrolador clasificador (adicionales):
```
Adafruit_PWMServoDriver
Adafruit_SSD1306
Adafruit_GFX
```

Para la cámara (adicionales):
```
arduino-base64
```

## 🔧 Configuración Hardware

### Microcontrolador UpCyclePRO
- **ESP32** con módulos: Motor, PIR sensors, Servos PCA9685, OLED
- **Conexiones:** Mantiene el esquema original del proyecto
- **Nuevas funcionalidades:** WiFi, MQTT, WebSocket, API REST

### Cámara ESP32-CAM  
- **ESP32-CAM** con sensor OV2640
- **Conexiones:** Estándar ESP32-CAM
- **Nuevas funcionalidades:** API REST, MQTT, WebSocket, análisis IA

## 🌐 Configuración de Red

### WiFi (Ambos dispositivos)
```cpp
const char* ssid = "TU_WIFI_SSID";
const char* password = "TU_WIFI_PASSWORD";
```

### MQTT Broker
```cpp
const char* mqtt_server = "IP_DE_NODERED";  // ej: "192.168.1.100"
const int mqtt_port = 1883;
```

## 📡 Protocolos de Comunicación

### 🔗 MQTT Topics

#### Microcontrolador
- `upcyclepro/command` (IN) - Recibe comandos
- `upcyclepro/status` (OUT) - Estado del sistema  
- `upcyclepro/sensor` (OUT) - Datos de sensores
- `upcyclepro/material` (OUT) - Material procesado

#### Cámara
- `camera/command` (IN) - Comandos de control
- `camera/status` (OUT) - Estado de la cámara
- `camera/image` (OUT) - Imágenes en base64

### 🌐 API REST

#### Microcontrolador (Puerto 80)
```
GET /status         - Estado actual del sistema
POST /classify      - Iniciar clasificación {"material": 1-3}
POST /stop          - Detener sistema
GET /sensors        - Lectura de sensores PIR
```

#### Cámara (Puerto 80)
```
GET /               - Página de información
GET /stream         - Video MJPEG stream
GET /capture        - Captura imagen JPEG
POST /analyze       - Captura + análisis IA
GET /status         - Estado de la cámara
POST /toggle_stream - Activar/desactivar streaming
```

### 🔌 WebSocket (Puerto 81)
- **Microcontrolador:** Datos de estado y sensores en tiempo real
- **Cámara:** Estado y comandos en tiempo real

## 🎮 Comandos JSON

### Microcontrolador
```json
// Clasificar material
{
  "action": "classify",
  "material": 1    // 1=vidrio, 2=plastico, 3=metal
}

// Detener sistema
{
  "action": "stop"
}

// Obtener estado
{
  "action": "get_status"
}
```

### Cámara
```json
// Capturar imagen
{
  "action": "capture"
}

// Iniciar streaming
{
  "action": "start_stream"
}

// Detener streaming  
{
  "action": "stop_stream"
}
```

## 📊 Respuestas del Sistema

### Estado del Microcontrolador
```json
{
  "device": "upcyclepro",
  "timestamp": 123456789,
  "state": "waiting",           // idle, waiting, processing
  "mode": 1,                    // 1, 2, 3 o 0
  "material": "vidrio",         // vidrio, plastico, metal, none
  "wifi_rssi": -45,
  "uptime": 3600
}
```

### Datos de Sensores
```json
{
  "device": "upcyclepro", 
  "timestamp": 123456789,
  "pir1": false,               // PIR Vidrio
  "pir2": true,                // PIR Plástico  
  "pir3": false,               // PIR Metal
  "motor": true                // Estado del motor
}
```

### Estado de la Cámara
```json
{
  "device": "upcyclepro_camera",
  "timestamp": 123456789,
  "streaming": true,
  "last_capture": 123456000,
  "wifi_rssi": -52,
  "free_heap": 234567,
  "ip": "192.168.1.101"
}
```

## 🚀 Instalación Node-RED

### 1. Instalar Node-RED
```bash
npm install -g node-red
```

### 2. Instalar Nodos Adicionales
```bash
cd ~/.node-red
npm install node-red-dashboard
npm install node-red-contrib-mqtt-broker
npm install node-red-contrib-websocket
```

### 3. Importar Flows
1. Copiar contenido de `flows_example.json`
2. En Node-RED: Menu → Import → Clipboard
3. Pegar contenido y hacer clic en Import

### 4. Configurar MQTT Broker
1. Editar nodo "Local MQTT"
2. Cambiar broker a IP local si es necesario
3. Deploy flows

## 🎯 Dashboard Node-RED

Acceder al dashboard en: `http://localhost:1880/ui`

### Secciones del Dashboard:
- **Control** - Botones para clasificar materiales
- **Estado del Sistema** - Estado actual y modo activo
- **Sensores** - Gauges de sensores PIR y motor
- **Cámara** - Stream de video en vivo
- **Análisis IA** - Resultados de clasificación automática
- **Estadísticas** - Gráficos de producción

## 🔄 Flujos de Trabajo

### 1. Clasificación Manual
```
Dashboard Button → MQTT Command → ESP32 → Classification → Status Update → Dashboard
```

### 2. Análisis Automático con Cámara
```
Timer → HTTP Capture → Image Analysis → Material Detection → Auto Classification
```

### 3. Monitoreo en Tiempo Real
```
ESP32 Sensors → MQTT → Node-RED → Dashboard Gauges/Charts
```

### 4. Integración Completa
```
Camera Detection → AI Analysis → Auto Command → Classification → Statistics → Alerts
```

## 🛠️ Troubleshooting

### ❌ ESP32 no conecta a WiFi
- Verificar SSID y password
- Revisar alcance de señal WiFi
- Cambiar canal del router si hay interferencia

### ❌ MQTT no conecta
- Verificar IP del broker MQTT
- Comprobar puerto 1883 abierto
- Revisar firewall local

### ❌ Dashboard no funciona
- Verificar instalación de node-red-dashboard
- Comprobar que flows estén deployed
- Revisar logs de Node-RED

### ❌ Cámara no stream
- Verificar alimentación ESP32-CAM
- Revisar conexiones del módulo cámara
- Comprobar memoria PSRAM disponible

## 🔍 Monitoreo y Logs

### Node-RED Debug
- Usar nodos debug para ver mensajes MQTT
- Monitor de Node-RED en `http://localhost:1880`

### ESP32 Serial Monitor
- Baudios: 115200
- Ver estado de conexiones y errores

### Dashboard Real-time
- Gráficos en tiempo real de sensores
- Alertas automáticas de fallos
- Estadísticas de rendimiento

## 🚀 Expansiones Futuras

### Integración IA
- Conexión con modelos TensorFlow Lite
- Clasificación automática por visión
- Aprendizaje continuo

### IoT Cloud
- Conexión con AWS IoT / Azure IoT
- Analytics en la nube  
- Alertas por email/SMS

### Base de Datos
- Registro histórico en InfluxDB
- Reportes de producción
- Análisis de tendencias

## 📞 Soporte

Para problemas técnicos:
1. Revisar logs en Serial Monitor
2. Verificar conectividad de red
3. Comprobar configuración MQTT
4. Consultar documentación de Node-RED

---

**¡Sistema UpCyclePro adaptado exitosamente para Node-RED! 🎉**