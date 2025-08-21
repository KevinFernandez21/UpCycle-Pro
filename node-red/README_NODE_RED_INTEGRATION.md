# 🌐 Node-RED Integration - UpCycle Pro

Esta guía explica cómo integrar Node-RED con el sistema UpCycle Pro para monitoreo, control y visualización de datos en tiempo real.

## 🎯 Objetivos de la Integración

1. **Dashboard en tiempo real** del sistema de clasificación
2. **Control remoto** del sistema (start/stop/cambio de modos)
3. **Monitoreo de sensores** PIR, motor y servos
4. **Estadísticas** de clasificación y rendimiento
5. **Alertas** automáticas por errores o timeouts
6. **Logging** histórico de operaciones
7. **API REST** para integración con otros sistemas

## 📋 Arquitectura de Integración

```
┌─────────────────┐    ┌──────────────┐    ┌─────────────────┐
│   ESP32         │    │   MQTT       │    │   Node-RED      │
│ (UpCycle Pro)   │◄──►│   Broker     │◄──►│   Dashboard     │
│                 │    │              │    │                 │
│ - PIR Sensors   │    │ - Messages   │    │ - UI Controls   │
│ - Motor         │    │ - Topics     │    │ - Charts        │
│ - Servos        │    │ - QoS        │    │ - Alerts        │
│ - OLED          │    │              │    │ - Database      │
└─────────────────┘    └──────────────┘    └─────────────────┘
           │                                         │
           │            ┌──────────────┐            │
           └────────────►│   Serial     │◄───────────┘
                        │   Monitor    │
                        └──────────────┘
```

## 🔧 Configuraciones Necesarias

### 1. ESP32 - Modificaciones de Código

#### A. Comunicación MQTT (Recomendado)
```cpp
// Agregar a SimpleClassifier.h
#include <WiFi.h>
#include <PubSubClient.h>

class SimpleClassifier {
private:
  // Existing members...
  WiFiClient wifiClient;
  PubSubClient mqttClient;
  
  // MQTT Topics
  const char* MQTT_TOPIC_STATUS = "upcycle/status";
  const char* MQTT_TOPIC_COMMAND = "upcycle/command";
  const char* MQTT_TOPIC_SENSORS = "upcycle/sensors";
  const char* MQTT_TOPIC_STATS = "upcycle/stats";
  
public:
  void initWiFi(const char* ssid, const char* password);
  void initMQTT(const char* broker, int port);
  void publishStatus();
  void publishSensorData();
  void handleMQTTCallback(char* topic, byte* payload, unsigned int length);
};
```

#### B. Comunicación Serial JSON (Alternativa)
```cpp
// Agregar a SimpleClassifier.h
#include <ArduinoJson.h>

public:
  void sendJSONStatus();
  void sendJSONSensorData();
  void processJSONCommand(String jsonCommand);
```

### 2. Node-RED - Configuración Inicial

#### Instalación de Nodos Necesarios:
```bash
# Nodos básicos para UpCycle Pro
npm install node-red-dashboard
npm install node-red-contrib-mqtt-broker
npm install node-red-node-serialport
npm install node-red-contrib-influxdb
npm install node-red-contrib-cronplus
npm install node-red-contrib-ui-led
```

## 📊 Flujos de Node-RED a Implementar

### 1. **Control Dashboard** 
- Botones para START/STOP sistema
- Selector de modo (VIDRIO/PLASTICO/METAL)
- Indicadores LED de estado
- Display de información en tiempo real

### 2. **Monitoreo de Sensores**
- Gráficos en tiempo real de PIR sensors
- Estado del motor (ON/OFF)
- Estado de servos (activaciones)
- Temperatura del sistema

### 3. **Estadísticas**
- Contador de objetos procesados por tipo
- Gráficos históricos de rendimiento
- Tiempo de funcionamiento
- Eficiencia del sistema

### 4. **Alertas y Notificaciones**
- Alertas por timeout del sistema
- Notificaciones de errores
- Alertas de mantenimiento
- Logs de eventos importantes

### 5. **API REST**
- Endpoints para control externo
- Consulta de estadísticas
- Configuración de parámetros
- Integración con otros sistemas

## 🧪 Tests a Implementar

### 1. **Test de Comunicación MQTT**
```javascript
// Flow Node-RED: MQTT Connection Test
[
    {
        "id": "mqtt-test",
        "type": "mqtt in",
        "broker": "localhost",
        "topic": "upcycle/test",
        "name": "MQTT Test Listener"
    }
]
```

### 2. **Test de Dashboard Básico**
- Crear dashboard simple con botones de control
- Probar envío de comandos al ESP32
- Verificar recepción de datos de estado

### 3. **Test de Visualización de Sensores**
- Gráficos en tiempo real de PIR sensors
- Indicadores de estado del motor
- Contadores de activaciones de servos

### 4. **Test de Persistencia de Datos**
- Configurar base de datos (InfluxDB o SQLite)
- Almacenar histórico de clasificaciones
- Consultas de datos históricos

### 5. **Test de Alertas**
- Configurar alertas por email/SMS
- Pruebas de notificaciones automáticas
- Logging de eventos críticos

## 🔌 Protocolo de Comunicación

### MQTT Topics Structure:
```
upcycle/
├── status/
│   ├── system          # Estado general del sistema
│   ├── motor           # Estado del motor
│   ├── servos          # Estado de servos
│   └── display         # Estado del display
├── sensors/
│   ├── pir1            # Sensor PIR VIDRIO
│   ├── pir2            # Sensor PIR PLASTICO
│   └── pir3            # Sensor PIR METAL
├── commands/
│   ├── mode            # Cambiar modo (1,2,3)
│   ├── start           # Iniciar clasificación
│   └── stop            # Detener sistema
├── stats/
│   ├── processed       # Objetos procesados
│   ├── efficiency      # Eficiencia del sistema
│   └── uptime          # Tiempo de funcionamiento
└── alerts/
    ├── errors          # Errores del sistema
    ├── timeouts        # Timeouts detectados
    └── maintenance     # Alertas de mantenimiento
```

### JSON Message Format:
```json
{
  "timestamp": "2025-01-13T10:30:00Z",
  "device_id": "upcycle_pro_001",
  "message_type": "status",
  "data": {
    "system_state": "WAITING",
    "selected_mode": 1,
    "motor_running": true,
    "pir_sensors": {
      "pir1": false,
      "pir2": false,
      "pir3": true
    },
    "servo_states": {
      "servo1": "idle",
      "servo2": "idle", 
      "servo3": "active"
    },
    "stats": {
      "objects_processed": 157,
      "vidrio_count": 45,
      "plastico_count": 62,
      "metal_count": 50,
      "uptime_minutes": 245
    }
  }
}
```

## 🎨 Dashboard Design

### Layout Propuesto:
```
┌─────────────────────────────────────────────────────────┐
│                 UPCYCLE PRO CONTROL CENTER              │
├─────────────────────────────────────────────────────────┤
│ CONTROL                          │ STATUS               │
│ ┌─────┐ ┌─────┐ ┌─────┐         │ System: ●RUNNING     │
│ │  1  │ │  2  │ │  3  │         │ Motor:  ●ON          │
│ │VIDRIO│PLASTIC│METAL │         │ Mode:   VIDRIO       │
│ └─────┘ └─────┘ └─────┘         │                      │
│ ┌────────┐ ┌──────────┐         │ PIR SENSORS          │
│ │ START  │ │   STOP   │         │ PIR1: ●  PIR2: ○     │
│ └────────┘ └──────────┘         │ PIR3: ○              │
├─────────────────────────────────────────────────────────┤
│ REAL-TIME MONITORING                                    │
│ ┌─────────────────────────────────────────────────────┐ │
│ │        OBJECTS PROCESSED (Last Hour)                │ │
│ │    50 ┤                                        ●    │ │
│ │    40 ┤                              ●         │    │ │
│ │    30 ┤                    ●         │         │    │ │
│ │    20 ┤          ●         │         │         │    │ │
│ │    10 ┤    ●     │         │         │         │    │ │
│ │     0 └─────┴─────┴─────┴─────┴─────┴─────┴────     │ │
│ └─────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────┤
│ STATISTICS                      │ ALERTS                │
│ Total Today:     234            │ ● System Running OK   │
│ Vidrio:         78 (33%)        │ ○ No active alerts    │
│ Plastico:       89 (38%)        │                       │
│ Metal:          67 (29%)        │ Last Error: None      │
│ Efficiency:     94.2%           │ Uptime: 4h 35m        │
└─────────────────────────────────────────────────────────┘
```

## 🚀 Pasos para Implementar

### Fase 1: Setup Básico (1-2 horas)
1. **Instalar Node-RED**
   ```bash
   npm install -g node-red
   node-red
   ```
   
2. **Configurar MQTT Broker**
   ```bash
   # Mosquitto (local)
   sudo apt install mosquitto mosquitto-clients
   mosquitto -d
   
   # O usar broker en la nube (HiveMQ, AWS IoT, etc.)
   ```

3. **Crear flow básico de prueba**
   - MQTT Input node
   - Debug node
   - Dashboard button

### Fase 2: Comunicación ESP32 (2-3 horas)
4. **Modificar código ESP32**
   - Agregar WiFi y MQTT
   - Implementar publishing de datos
   - Agregar subscriber para comandos

5. **Probar comunicación bidireccional**
   - ESP32 → Node-RED (datos)
   - Node-RED → ESP32 (comandos)

### Fase 3: Dashboard Completo (3-4 horas)
6. **Crear dashboard UI**
   - Control buttons
   - Status indicators
   - Real-time charts
   - Statistics display

7. **Implementar lógica de control**
   - Command processing
   - Data validation
   - Error handling

### Fase 4: Características Avanzadas (4-6 horas)
8. **Base de datos y persistencia**
   - InfluxDB setup
   - Historical data storage
   - Query implementation

9. **Alertas y notificaciones**
   - Email alerts
   - SMS notifications
   - Dashboard alerts

10. **API REST**
    - HTTP endpoints
    - Authentication
    - External integration

## 🔍 Tests Específicos a Realizar

### 1. **Test de Latencia**
```javascript
// Medir tiempo de respuesta ESP32 ↔ Node-RED
const startTime = Date.now();
// Enviar comando
// Recibir respuesta
const latency = Date.now() - startTime;
```

### 2. **Test de Throughput**
- Enviar múltiples comandos seguidos
- Verificar que no se pierdan mensajes
- Medir velocidad de procesamiento

### 3. **Test de Confiabilidad**
- Desconectar/reconectar WiFi
- Reiniciar ESP32
- Verificar recuperación automática

### 4. **Test de Escalabilidad**
- Múltiples clientes conectados
- Alto volumen de datos
- Performance del dashboard

### 5. **Test de Seguridad**
- Autenticación MQTT
- Encriptación de mensajes
- Control de acceso

## 📚 Recursos y Documentación

### Node-RED:
- [Documentación oficial](https://nodered.org/docs/)
- [Dashboard nodes](https://flows.nodered.org/node/node-red-dashboard)
- [MQTT integration](https://cookbook.nodered.org/mqtt/)

### ESP32 MQTT:
- [PubSubClient library](https://github.com/knolleary/pubsubclient)
- [WiFi library](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi)
- [ArduinoJSON](https://arduinojson.org/)

### Ejemplos de Flows:
- Industrial IoT dashboards
- Sensor monitoring systems
- Remote control applications

## 🎉 Beneficios Esperados

1. **Monitoreo remoto** del sistema UpCycle Pro
2. **Control centralizado** desde cualquier dispositivo
3. **Análisis de datos** históricos y en tiempo real
4. **Alertas automáticas** para mantenimiento preventivo
5. **Integración** fácil con otros sistemas
6. **Dashboard profesional** para demostración
7. **Escalabilidad** para múltiples sistemas

¡Con Node-RED podrás convertir tu sistema UpCycle Pro en una solución IoT completa y profesional! 🚀