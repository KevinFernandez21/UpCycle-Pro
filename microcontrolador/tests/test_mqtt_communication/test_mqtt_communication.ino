/*
  TEST MQTT COMMUNICATION - UpCycle Pro + Node-RED
  =================================================
  
  Este sketch prueba la comunicación MQTT entre el ESP32 y Node-RED
  para integrar el sistema UpCycle Pro con un dashboard web.
  
  Características:
  - Conexión WiFi automática
  - Comunicación MQTT bidireccional
  - Publicación de datos de sensores y estado
  - Suscripción a comandos desde Node-RED
  - Protocolo JSON estructurado
  
  Temas MQTT utilizados:
  - upcycle/status/system    (ESP32 → Node-RED)
  - upcycle/status/motor     (ESP32 → Node-RED)  
  - upcycle/sensors/pir1     (ESP32 → Node-RED)
  - upcycle/sensors/pir2     (ESP32 → Node-RED)
  - upcycle/sensors/pir3     (ESP32 → Node-RED)
  - upcycle/command/mode     (Node-RED → ESP32)
  - upcycle/command/system   (Node-RED → ESP32)
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Incluir las librerías del proyecto
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/PIRManager.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/PIRManager.cpp"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ConveyorMotor.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ConveyorMotor.cpp"

// ===============================================================================
// CONFIGURACIÓN WIFI Y MQTT
// ===============================================================================

// Configuración WiFi - CAMBIAR SEGÚN TU RED
const char* WIFI_SSID = "Tu_Red_WiFi";
const char* WIFI_PASSWORD = "Tu_Password_WiFi";

// Configuración MQTT - CAMBIAR SEGÚN TU BROKER
const char* MQTT_BROKER = "localhost";  // O IP del broker MQTT
const int MQTT_PORT = 1883;
const char* MQTT_CLIENT_ID = "upcycle_esp32";

// Temas MQTT
const char* TOPIC_STATUS_SYSTEM = "upcycle/status/system";
const char* TOPIC_STATUS_MOTOR = "upcycle/status/motor";
const char* TOPIC_SENSORS_PIR1 = "upcycle/sensors/pir1";
const char* TOPIC_SENSORS_PIR2 = "upcycle/sensors/pir2";
const char* TOPIC_SENSORS_PIR3 = "upcycle/sensors/pir3";
const char* TOPIC_COMMAND_MODE = "upcycle/command/mode";
const char* TOPIC_COMMAND_SYSTEM = "upcycle/command/system";

// ===============================================================================
// OBJETOS GLOBALES
// ===============================================================================

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
PIRManager pirSensors;
ConveyorMotor motor;

// ===============================================================================
// VARIABLES DE ESTADO
// ===============================================================================

enum SystemState {
  SYSTEM_IDLE,
  SYSTEM_WAITING,
  SYSTEM_PROCESSING
};

SystemState currentState = SYSTEM_IDLE;
int selectedMode = 0; // 0=NONE, 1=VIDRIO, 2=PLASTICO, 3=METAL
bool systemInitialized = false;

// Variables de timing
unsigned long lastStatusPublish = 0;
unsigned long lastSensorPublish = 0;
unsigned long lastHeartbeat = 0;
const unsigned long STATUS_INTERVAL = 2000;    // 2 segundos
const unsigned long SENSOR_INTERVAL = 1000;    // 1 segundo
const unsigned long HEARTBEAT_INTERVAL = 30000; // 30 segundos

// Contadores de mensajes
unsigned long messagesSent = 0;
unsigned long messagesReceived = 0;
unsigned long mqttReconnects = 0;

// ===============================================================================
// SETUP
// ===============================================================================

void setup() {
  Serial.begin(115200);
  
  Serial.println();
  Serial.println("🌐 TEST MQTT COMMUNICATION - UpCycle Pro");
  Serial.println("=========================================");
  Serial.println();
  
  // Inicializar hardware
  if (!initializeHardware()) {
    Serial.println("❌ Error inicializando hardware");
    while(1) delay(1000);
  }
  
  // Conectar WiFi
  if (!connectWiFi()) {
    Serial.println("❌ Error conectando WiFi");
    while(1) delay(1000);
  }
  
  // Configurar MQTT
  setupMQTT();
  
  // Conectar MQTT
  if (!connectMQTT()) {
    Serial.println("❌ Error conectando MQTT");
    while(1) delay(1000);
  }
  
  systemInitialized = true;
  currentState = SYSTEM_IDLE;
  
  Serial.println("✅ Sistema MQTT inicializado correctamente");
  Serial.println("🔄 Publicando datos cada 2 segundos...");
  Serial.println("📱 Abre el dashboard de Node-RED en http://broker_ip:1880/ui");
  Serial.println();
  
  printMQTTHelp();
}

// ===============================================================================
// LOOP PRINCIPAL
// ===============================================================================

void loop() {
  // Mantener conexión MQTT
  if (!mqttClient.connected()) {
    if (connectMQTT()) {
      mqttReconnects++;
    } else {
      delay(5000);
      return;
    }
  }
  mqttClient.loop();
  
  // Procesar comandos seriales locales
  if (Serial.available()) {
    processSerialCommand();
  }
  
  // Leer sensores
  readSensors();
  
  // Publicar datos periódicamente
  if (millis() - lastStatusPublish > STATUS_INTERVAL) {
    publishSystemStatus();
    publishMotorStatus();
    lastStatusPublish = millis();
  }
  
  if (millis() - lastSensorPublish > SENSOR_INTERVAL) {
    publishSensorData();
    lastSensorPublish = millis();
  }
  
  // Heartbeat
  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    publishHeartbeat();
    lastHeartbeat = millis();
  }
  
  delay(100);
}

// ===============================================================================
// INICIALIZACIÓN
// ===============================================================================

bool initializeHardware() {
  Serial.println("🔄 Inicializando hardware...");
  
  // Inicializar PIRManager
  pirSensors.begin();
  Serial.println("✅ PIRManager inicializado");
  
  // Inicializar ConveyorMotor
  motor.begin();
  Serial.println("✅ ConveyorMotor inicializado");
  
  delay(2000); // Estabilización
  return true;
}

bool connectWiFi() {
  Serial.println("🔄 Conectando a WiFi...");
  Serial.print("Red: ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ WiFi conectado");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("❌ Error conectando WiFi");
    return false;
  }
}

void setupMQTT() {
  Serial.println("🔄 Configurando MQTT...");
  Serial.print("Broker: ");
  Serial.print(MQTT_BROKER);
  Serial.print(":");
  Serial.println(MQTT_PORT);
  
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(onMqttMessage);
  mqttClient.setBufferSize(1024); // Buffer más grande para JSON
}

bool connectMQTT() {
  Serial.println("🔄 Conectando MQTT...");
  
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("✅ MQTT conectado");
    
    // Suscribirse a comandos
    mqttClient.subscribe(TOPIC_COMMAND_MODE, 1);
    mqttClient.subscribe(TOPIC_COMMAND_SYSTEM, 1);
    
    Serial.println("📝 Suscrito a topics de comando:");
    Serial.println("  - " + String(TOPIC_COMMAND_MODE));
    Serial.println("  - " + String(TOPIC_COMMAND_SYSTEM));
    
    // Publicar mensaje de conexión
    publishConnectionStatus(true);
    
    return true;
  } else {
    Serial.print("❌ Error MQTT: ");
    Serial.println(mqttClient.state());
    return false;
  }
}

// ===============================================================================
// MQTT CALLBACKS Y COMANDOS
// ===============================================================================

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  messagesReceived++;
  
  // Convertir payload a string
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.println("📨 MQTT recibido:");
  Serial.println("  Topic: " + String(topic));
  Serial.println("  Message: " + message);
  
  // Procesar comandos
  if (String(topic) == TOPIC_COMMAND_MODE) {
    handleModeCommand(message);
  } else if (String(topic) == TOPIC_COMMAND_SYSTEM) {
    handleSystemCommand(message);
  }
}

void handleModeCommand(String mode) {
  int modeNum = mode.toInt();
  
  if (modeNum >= 1 && modeNum <= 3) {
    selectedMode = modeNum;
    currentState = SYSTEM_WAITING;
    
    String material = "";
    switch(modeNum) {
      case 1: material = "VIDRIO"; break;
      case 2: material = "PLASTICO"; break;
      case 3: material = "METAL"; break;
    }
    
    motor.start();
    
    Serial.println("🎯 Modo cambiado vía MQTT: " + String(modeNum) + " (" + material + ")");
    
    // Confirmar comando
    publishModeConfirmation(modeNum, material);
    
  } else {
    Serial.println("❌ Modo MQTT inválido: " + mode);
  }
}

void handleSystemCommand(String command) {
  if (command == "stop") {
    motor.stop();
    currentState = SYSTEM_IDLE;
    selectedMode = 0;
    
    Serial.println("🛑 Sistema detenido vía MQTT");
    
    // Confirmar comando
    publishSystemStopConfirmation();
    
  } else if (command == "status") {
    // Publicar estado inmediatamente
    publishSystemStatus();
    publishMotorStatus();
    publishSensorData();
    
    Serial.println("📊 Estado publicado vía MQTT");
    
  } else {
    Serial.println("❌ Comando MQTT desconocido: " + command);
  }
}

// ===============================================================================
// PUBLICACIÓN DE DATOS
// ===============================================================================

void publishSystemStatus() {
  JsonDocument doc;
  
  doc["timestamp"] = millis();
  doc["device_id"] = MQTT_CLIENT_ID;
  
  // Estado del sistema
  switch(currentState) {
    case SYSTEM_IDLE:
      doc["state"] = "IDLE";
      break;
    case SYSTEM_WAITING:
      doc["state"] = "WAITING";
      break;
    case SYSTEM_PROCESSING:
      doc["state"] = "PROCESSING";
      break;
  }
  
  doc["mode"] = selectedMode;
  
  String material = "NONE";
  switch(selectedMode) {
    case 1: material = "VIDRIO"; break;
    case 2: material = "PLASTICO"; break;
    case 3: material = "METAL"; break;
  }
  doc["material"] = material;
  
  doc["uptime_ms"] = millis();
  doc["free_heap"] = ESP.getFreeHeap();
  doc["wifi_rssi"] = WiFi.RSSI();
  
  String payload;
  serializeJson(doc, payload);
  
  if (mqttClient.publish(TOPIC_STATUS_SYSTEM, payload.c_str(), true)) {
    messagesSent++;
  } else {
    Serial.println("❌ Error publicando estado del sistema");
  }
}

void publishMotorStatus() {
  JsonDocument doc;
  
  doc["timestamp"] = millis();
  doc["running"] = motor.isMotorRunning();
  doc["state"] = motor.isMotorRunning() ? "ON" : "OFF";
  
  String payload;
  serializeJson(doc, payload);
  
  if (mqttClient.publish(TOPIC_STATUS_MOTOR, payload.c_str(), true)) {
    messagesSent++;
  } else {
    Serial.println("❌ Error publicando estado del motor");
  }
}

void publishSensorData() {
  // Leer estados actuales de PIR
  bool pir1 = digitalRead(26); // PIR1_PIN
  bool pir2 = digitalRead(27); // PIR2_PIN 
  bool pir3 = digitalRead(14); // PIR3_PIN
  
  // PIR 1 (VIDRIO)
  JsonDocument doc1;
  doc1["timestamp"] = millis();
  doc1["sensor"] = "PIR1";
  doc1["material"] = "VIDRIO";
  doc1["active"] = pir1;
  doc1["gpio"] = 26;
  
  String payload1;
  serializeJson(doc1, payload1);
  mqttClient.publish(TOPIC_SENSORS_PIR1, payload1.c_str(), false);
  
  // PIR 2 (PLASTICO)
  JsonDocument doc2;
  doc2["timestamp"] = millis();
  doc2["sensor"] = "PIR2";
  doc2["material"] = "PLASTICO";
  doc2["active"] = pir2;
  doc2["gpio"] = 27;
  
  String payload2;
  serializeJson(doc2, payload2);
  mqttClient.publish(TOPIC_SENSORS_PIR2, payload2.c_str(), false);
  
  // PIR 3 (METAL)
  JsonDocument doc3;
  doc3["timestamp"] = millis();
  doc3["sensor"] = "PIR3";
  doc3["material"] = "METAL";
  doc3["active"] = pir3;
  doc3["gpio"] = 14;
  
  String payload3;
  serializeJson(doc3, payload3);
  mqttClient.publish(TOPIC_SENSORS_PIR3, payload3.c_str(), false);
  
  messagesSent += 3;
}

void publishModeConfirmation(int mode, String material) {
  JsonDocument doc;
  
  doc["timestamp"] = millis();
  doc["command"] = "mode_changed";
  doc["mode"] = mode;
  doc["material"] = material;
  doc["status"] = "confirmed";
  
  String payload;
  serializeJson(doc, payload);
  
  mqttClient.publish("upcycle/response/mode", payload.c_str(), false);
  messagesSent++;
}

void publishSystemStopConfirmation() {
  JsonDocument doc;
  
  doc["timestamp"] = millis();
  doc["command"] = "system_stopped";
  doc["status"] = "confirmed";
  
  String payload;
  serializeJson(doc, payload);
  
  mqttClient.publish("upcycle/response/system", payload.c_str(), false);
  messagesSent++;
}

void publishConnectionStatus(bool connected) {
  JsonDocument doc;
  
  doc["timestamp"] = millis();
  doc["device_id"] = MQTT_CLIENT_ID;
  doc["connected"] = connected;
  doc["ip_address"] = WiFi.localIP().toString();
  doc["mac_address"] = WiFi.macAddress();
  
  String payload;
  serializeJson(doc, payload);
  
  mqttClient.publish("upcycle/connection", payload.c_str(), true);
  messagesSent++;
}

void publishHeartbeat() {
  JsonDocument doc;
  
  doc["timestamp"] = millis();
  doc["device_id"] = MQTT_CLIENT_ID;
  doc["uptime_ms"] = millis();
  doc["messages_sent"] = messagesSent;
  doc["messages_received"] = messagesReceived;
  doc["mqtt_reconnects"] = mqttReconnects;
  doc["free_heap"] = ESP.getFreeHeap();
  doc["wifi_rssi"] = WiFi.RSSI();
  
  String payload;
  serializeJson(doc, payload);
  
  mqttClient.publish("upcycle/heartbeat", payload.c_str(), false);
  messagesSent++;
  
  Serial.print("💓 Heartbeat - Sent: ");
  Serial.print(messagesSent);
  Serial.print(" | Received: ");
  Serial.print(messagesReceived);
  Serial.print(" | Reconnects: ");
  Serial.println(mqttReconnects);
}

// ===============================================================================
// CONTROL LOCAL
// ===============================================================================

void readSensors() {
  // Verificar detecciones usando PIRManager si está en modo espera
  if (currentState == SYSTEM_WAITING) {
    bool detected = false;
    String material = "";
    
    switch(selectedMode) {
      case 1: // VIDRIO
        if (pirSensors.checkPIR1()) {
          Serial.println("🎯 VIDRIO detectado (MQTT mode)");
          material = "VIDRIO";
          detected = true;
        }
        break;
      case 2: // PLASTICO  
        if (pirSensors.checkPIR2()) {
          Serial.println("🎯 PLASTICO detectado (MQTT mode)");
          material = "PLASTICO";
          detected = true;
        }
        break;
      case 3: // METAL
        if (pirSensors.checkPIR3()) {
          Serial.println("🎯 METAL detectado (MQTT mode)");
          material = "METAL";
          detected = true;
        }
        break;
    }
    
    if (detected) {
      currentState = SYSTEM_PROCESSING;
      motor.stop();
      
      // Publicar detección
      publishDetectionEvent(material);
      
      delay(2000); // Simular procesamiento
      
      // Volver a espera
      currentState = SYSTEM_WAITING;
      motor.start();
      
      Serial.println("✅ Procesamiento completado (MQTT mode)");
    }
  }
}

void publishDetectionEvent(String material) {
  JsonDocument doc;
  
  doc["timestamp"] = millis();
  doc["event"] = "detection";
  doc["material"] = material;
  doc["mode"] = selectedMode;
  
  String payload;
  serializeJson(doc, payload);
  
  mqttClient.publish("upcycle/events/detection", payload.c_str(), false);
  messagesSent++;
}

void processSerialCommand() {
  String command = Serial.readString();
  command.trim();
  command.toLowerCase();
  
  if (command == "status" || command == "s") {
    printSystemStatus();
  } else if (command == "mqtt" || command == "m") {
    printMQTTStatus();
  } else if (command == "test" || command == "t") {
    runMQTTTest();
  } else if (command == "help" || command == "h") {
    printMQTTHelp();
  } else if (command.startsWith("mode ")) {
    int mode = command.substring(5).toInt();
    if (mode >= 1 && mode <= 3) {
      handleModeCommand(String(mode));
    }
  } else if (command == "stop") {
    handleSystemCommand("stop");
  }
}

// ===============================================================================
// INFORMACIÓN Y DIAGNÓSTICO
// ===============================================================================

void printSystemStatus() {
  Serial.println("📋 ESTADO DEL SISTEMA");
  Serial.println("=====================");
  
  String stateStr = "";
  switch(currentState) {
    case SYSTEM_IDLE: stateStr = "IDLE"; break;
    case SYSTEM_WAITING: stateStr = "WAITING"; break;
    case SYSTEM_PROCESSING: stateStr = "PROCESSING"; break;
  }
  
  Serial.println("Estado: " + stateStr);
  Serial.println("Modo: " + String(selectedMode));
  Serial.println("Motor: " + String(motor.isMotorRunning() ? "ON" : "OFF"));
  Serial.println("WiFi: " + String(WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado"));
  Serial.println("MQTT: " + String(mqttClient.connected() ? "Conectado" : "Desconectado"));
  Serial.println();
}

void printMQTTStatus() {
  Serial.println("📡 ESTADO MQTT");
  Serial.println("==============");
  Serial.println("Broker: " + String(MQTT_BROKER) + ":" + String(MQTT_PORT));
  Serial.println("Cliente: " + String(MQTT_CLIENT_ID));
  Serial.println("Conectado: " + String(mqttClient.connected() ? "SÍ" : "NO"));
  Serial.println("Estado: " + String(mqttClient.state()));
  Serial.println();
  Serial.println("📊 Estadísticas:");
  Serial.println("Mensajes enviados: " + String(messagesSent));
  Serial.println("Mensajes recibidos: " + String(messagesReceived));
  Serial.println("Reconexiones: " + String(mqttReconnects));
  Serial.println("Heap libre: " + String(ESP.getFreeHeap()) + " bytes");
  Serial.println("RSSI WiFi: " + String(WiFi.RSSI()) + " dBm");
  Serial.println();
}

void runMQTTTest() {
  Serial.println("🧪 EJECUTANDO TEST MQTT");
  Serial.println("========================");
  
  if (!mqttClient.connected()) {
    Serial.println("❌ MQTT no conectado - no se puede hacer test");
    return;
  }
  
  // Test 1: Publicar estado
  Serial.println("1️⃣ Publicando estado del sistema...");
  publishSystemStatus();
  delay(1000);
  
  // Test 2: Publicar sensores
  Serial.println("2️⃣ Publicando datos de sensores...");
  publishSensorData();
  delay(1000);
  
  // Test 3: Simular detección
  Serial.println("3️⃣ Simulando detección de VIDRIO...");
  publishDetectionEvent("VIDRIO");
  delay(1000);
  
  // Test 4: Heartbeat
  Serial.println("4️⃣ Enviando heartbeat...");
  publishHeartbeat();
  delay(1000);
  
  Serial.println("✅ Test MQTT completado");
  Serial.println("📊 Verifica los mensajes en Node-RED Debug");
  Serial.println();
}

void printMQTTHelp() {
  Serial.println("🎮 COMANDOS DISPONIBLES (Serial):");
  Serial.println("=================================");
  Serial.println("- 'status' o 's' = Estado del sistema");
  Serial.println("- 'mqtt' o 'm' = Estado MQTT y estadísticas");
  Serial.println("- 'test' o 't' = Ejecutar test MQTT");
  Serial.println("- 'mode 1/2/3' = Cambiar modo localmente");
  Serial.println("- 'stop' = Detener sistema");
  Serial.println("- 'help' o 'h' = Mostrar esta ayuda");
  Serial.println();
  Serial.println("🌐 CONTROL REMOTO (Node-RED):");
  Serial.println("==============================");
  Serial.println("Dashboard: http://" + String(MQTT_BROKER) + ":1880/ui");
  Serial.println("Topics de comando:");
  Serial.println("  - " + String(TOPIC_COMMAND_MODE) + " → Enviar '1', '2' o '3'");
  Serial.println("  - " + String(TOPIC_COMMAND_SYSTEM) + " → Enviar 'stop'");
  Serial.println();
  Serial.println("Topics de estado (automáticos):");
  Serial.println("  - " + String(TOPIC_STATUS_SYSTEM));
  Serial.println("  - " + String(TOPIC_STATUS_MOTOR));  
  Serial.println("  - " + String(TOPIC_SENSORS_PIR1));
  Serial.println("  - " + String(TOPIC_SENSORS_PIR2));
  Serial.println("  - " + String(TOPIC_SENSORS_PIR3));
  Serial.println();
}

/*
===============================================================================
                                MANUAL DE USO
===============================================================================

🎯 OBJETIVO:
Probar la comunicación MQTT entre ESP32 y Node-RED para control remoto
y monitoreo del sistema UpCycle Pro.

🔧 CONFIGURACIÓN PREVIA:

1. CONFIGURAR WIFI:
   - Cambiar WIFI_SSID y WIFI_PASSWORD en el código
   - Asegurarse que ESP32 esté en la misma red que Node-RED

2. INSTALAR MQTT BROKER:
   # Mosquitto (Linux/Mac)
   sudo apt install mosquitto mosquitto-clients
   mosquitto -d
   
   # Windows - Descargar de mosquitto.org
   # O usar broker en la nube (HiveMQ Cloud, AWS IoT, etc.)

3. CONFIGURAR NODE-RED:
   npm install -g node-red
   cd node-red-directory
   npm install node-red-dashboard node-red-contrib-mqtt-broker
   node-red
   
4. IMPORTAR FLOW:
   - Abrir http://localhost:1880
   - Import → Clipboard
   - Pegar contenido de basic_dashboard.json

📋 PROCEDIMIENTO DE PRUEBA:

FASE 1 - CONEXIÓN BÁSICA:
1. Subir sketch al ESP32
2. Abrir Monitor Serial (115200 baud)
3. Verificar conexión WiFi y MQTT
4. Comprobar que aparezcan mensajes de publicación

FASE 2 - DASHBOARD NODE-RED:
1. Abrir dashboard: http://localhost:1880/ui
2. Verificar que aparezcan indicadores de estado
3. Probar botones de control (VIDRIO, PLASTICO, METAL, STOP)
4. Verificar que comandos lleguen al ESP32

FASE 3 - MONITOREO EN TIEMPO REAL:
1. Monitor Serial debe mostrar comandos recibidos de Node-RED
2. Dashboard debe mostrar estado del motor y sensores PIR
3. LEDs deben cambiar según estado del sistema

FASE 4 - TEST COMPLETO:
1. Enviar comando 'test' en Serial
2. Verificar mensajes en Debug de Node-RED
3. Probar todos los modos desde dashboard
4. Verificar detecciones de PIR en tiempo real

🎮 COMANDOS DE PRUEBA:

DESDE MONITOR SERIAL:
- 'status': Ver estado completo del sistema
- 'mqtt': Ver estadísticas de MQTT
- 'test': Ejecutar test automático
- 'mode 1': Cambiar a modo VIDRIO localmente
- 'stop': Detener sistema

DESDE NODE-RED DASHBOARD:
- Botón "🔵 VIDRIO": Envía comando modo 1
- Botón "🟠 PLASTICO": Envía comando modo 2  
- Botón "⚫ METAL": Envía comando modo 3
- Botón "🛑 STOP": Detiene el sistema

✅ RESULTADOS ESPERADOS:

CONEXIONES:
- WiFi conectado con IP asignada
- MQTT conectado sin errores
- Node-RED recibiendo datos cada 2 segundos

COMUNICACIÓN BIDIRECCIONAL:
- ESP32 → Node-RED: Estado, motor, sensores
- Node-RED → ESP32: Comandos de modo y control

DASHBOARD FUNCIONAL:
- Indicadores LED cambian según estado real
- Botones envían comandos correctamente
- Datos se actualizan en tiempo real

❌ PROBLEMAS COMUNES:

"WiFi no conecta":
- Verificar SSID y password correctos
- Verificar que red esté disponible
- Comprobar señal WiFi suficiente

"MQTT no conecta":
- Verificar que broker esté funcionando
- Comprobar IP/puerto del broker
- Verificar firewall no bloquee puerto 1883

"Node-RED no recibe datos":
- Verificar configuración de broker en flow
- Comprobar topics correctos
- Revisar Debug nodes en Node-RED

"Dashboard no responde":
- Verificar instalación node-red-dashboard
- Comprobar que flow esté desplegado
- Abrir http://IP:1880/ui correcto

📊 MONITOREO:

TOPICS MQTT:
- upcycle/status/system: Estado general
- upcycle/status/motor: Estado del motor
- upcycle/sensors/pir1-3: Sensores PIR
- upcycle/command/mode: Comandos de modo
- upcycle/command/system: Comandos del sistema

ESTADÍSTICAS:
- Mensajes enviados/recibidos
- Reconexiones MQTT
- Memoria libre ESP32
- Señal WiFi (RSSI)

🚀 SIGUIENTE PASO:
Una vez funcionando correctamente, integrar con SimpleClassifier
para tener control completo del sistema vía Node-RED.

===============================================================================
*/