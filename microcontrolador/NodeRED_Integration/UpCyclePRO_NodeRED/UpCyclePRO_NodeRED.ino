/*
  CLASIFICADORA DE MATERIALES - INTEGRACIÓN NODE-RED
  =================================================
  
  Adaptación del sistema UpCyclePRO para Node-RED:
  - Comunicación WiFi + MQTT
  - API REST para control remoto
  - WebSocket para datos en tiempo real
  - JSON para todos los mensajes
  
  Tópicos MQTT:
  - upcyclepro/command (IN) - Recibe comandos
  - upcyclepro/status (OUT) - Estado del sistema
  - upcyclepro/sensor (OUT) - Datos de sensores
  - upcyclepro/material (OUT) - Material procesado
  
  API REST:
  - GET /status - Estado actual
  - POST /classify - Enviar comando de clasificación
  - GET /sensors - Lectura de sensores
  - POST /stop - Detener sistema
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "SimpleClassifier.h"

// Configuración WiFi
const char* ssid = "hnaranja21";
const char* password = "hola1234";

// Configuración MQTT
const char* mqtt_server = "192.168.137.1";
const int mqtt_port = 1880;
const char* mqtt_client_id = "upcyclepro_esp32";

// Tópicos MQTT
const char* topic_command = "upcyclepro/command";
const char* topic_status = "upcyclepro/status";
const char* topic_sensor = "upcyclepro/sensor";
const char* topic_material = "upcyclepro/material";

// Instancias
WiFiClient espClient;
PubSubClient mqtt(espClient);
WebServer webServer(80);
WebSocketsServer webSocket(81);
SimpleClassifier classifier;

// Variables de estado
unsigned long lastStatusSend = 0;
unsigned long lastSensorRead = 0;
const unsigned long STATUS_INTERVAL = 5000;   // 5 segundos
const unsigned long SENSOR_INTERVAL = 1000;   // 1 segundo

void setup() {
  Serial.begin(115200);
  
  // Inicializar clasificador
  if (!classifier.begin()) {
    Serial.println("❌ Error inicializando sistema");
    while(1) delay(1000);
  }
  
  // Conectar WiFi
  setupWiFi();
  
  // Configurar MQTT
  mqtt.setServer(mqtt_server, mqtt_port);
  mqtt.setCallback(onMqttMessage);
  
  // Configurar servidor web
  setupWebServer();
  
  // Configurar WebSocket
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  
  Serial.println("🚀 Sistema Node-RED listo");
  sendStatusUpdate();
}

void loop() {
  // Mantener conexiones
  if (!mqtt.connected()) {
    reconnectMQTT();
  }
  mqtt.loop();
  
  webServer.handleClient();
  webSocket.loop();
  
  // Actualizar clasificador
  classifier.update();
  
  // Enviar estado periódicamente
  if (millis() - lastStatusSend > STATUS_INTERVAL) {
    sendStatusUpdate();
    lastStatusSend = millis();
  }
  
  // Enviar datos de sensores
  if (millis() - lastSensorRead > SENSOR_INTERVAL) {
    sendSensorData();
    lastSensorRead = millis();
  }
  
  delay(100);
}

void setupWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("📶 WiFi conectado - IP: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("🔄 Conectando MQTT...");
    
    if (mqtt.connect(mqtt_client_id)) {
      Serial.println(" ✅ Conectado");
      mqtt.subscribe(topic_command);
      sendStatusUpdate();
    } else {
      Serial.print(" ❌ Error: ");
      Serial.println(mqtt.state());
      delay(5000);
    }
  }
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.println("📨 MQTT: " + message);
  
  // Parsear JSON
  StaticJsonDocument<200> doc;
  if (deserializeJson(doc, message) == DeserializationError::Ok) {
    handleCommand(doc);
  }
}

void handleCommand(JsonDocument& cmd) {
  if (cmd.containsKey("action")) {
    String action = cmd["action"];
    
    if (action == "classify") {
      int material = cmd["material"]; // 1=vidrio, 2=plastico, 3=metal
      classifier.setMode(getMaterialString(material));
      
      // Responder confirmación
      StaticJsonDocument<200> response;
      response["action"] = "classify";
      response["material"] = material;
      response["status"] = "started";
      response["timestamp"] = millis();
      publishJson(topic_status, response);
      
    } else if (action == "stop") {
      classifier.stopSystem();
      
      StaticJsonDocument<200> response;
      response["action"] = "stop";
      response["status"] = "stopped";
      response["timestamp"] = millis();
      publishJson(topic_status, response);
      
    } else if (action == "get_status") {
      sendStatusUpdate();
    }
  }
}

void sendStatusUpdate() {
  StaticJsonDocument<300> status;
  
  status["device"] = "upcyclepro";
  status["timestamp"] = millis();
  status["state"] = getStateString(classifier.getState());
  status["mode"] = classifier.getMode();
  status["material"] = getMaterialString(classifier.getMode());
  status["wifi_rssi"] = WiFi.RSSI();
  status["uptime"] = millis() / 1000;
  
  publishJson(topic_status, status);
  broadcastWebSocket(status);
}

void sendSensorData() {
  StaticJsonDocument<200> sensors;
  
  sensors["device"] = "upcyclepro";
  sensors["timestamp"] = millis();
  sensors["pir1"] = digitalRead(26);  // PIR Vidrio
  sensors["pir2"] = digitalRead(27);  // PIR Plástico
  sensors["pir3"] = digitalRead(14);  // PIR Metal
  sensors["motor"] = classifier.getState() == WAITING ? true : false;
  
  publishJson(topic_sensor, sensors);
}

void publishJson(const char* topic, JsonDocument& doc) {
  String output;
  serializeJson(doc, output);
  mqtt.publish(topic, output.c_str());
}

void broadcastWebSocket(JsonDocument& doc) {
  String output;
  serializeJson(doc, output);
  webSocket.broadcastTXT(output);
}

String getStateString(SystemState state) {
  switch(state) {
    case IDLE: return "idle";
    case WAITING: return "waiting";
    case PROCESSING: return "processing";
    default: return "unknown";
  }
}

String getMaterialString(int mode) {
  switch(mode) {
    case 1: return "vidrio";
    case 2: return "plastico";
    case 3: return "metal";
    default: return "none";
  }
}

void setupWebServer() {
  // CORS headers
  webServer.onNotFound([]() {
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    webServer.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    
    if (webServer.method() == HTTP_OPTIONS) {
      webServer.send(200);
      return;
    }
    webServer.send(404, "text/plain", "Not Found");
  });
  
  // GET /status - Estado actual
  webServer.on("/status", HTTP_GET, []() {
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    
    StaticJsonDocument<300> response;
    response["device"] = "upcyclepro";
    response["timestamp"] = millis();
    response["state"] = getStateString(classifier.getState());
    response["mode"] = classifier.getMode();
    response["material"] = getMaterialString(classifier.getMode());
    response["ip"] = WiFi.localIP().toString();
    
    String output;
    serializeJson(response, output);
    webServer.send(200, "application/json", output);
  });
  
  // POST /classify - Clasificar material
  webServer.on("/classify", HTTP_POST, []() {
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    
    if (webServer.hasArg("plain")) {
      StaticJsonDocument<200> doc;
      if (deserializeJson(doc, webServer.arg("plain")) == DeserializationError::Ok) {
        
        if (doc.containsKey("material")) {
          int material = doc["material"];
          classifier.setMode(getMaterialString(material));
          
          StaticJsonDocument<200> response;
          response["action"] = "classify";
          response["material"] = material;
          response["material_name"] = getMaterialString(material);
          response["status"] = "started";
          response["timestamp"] = millis();
          
          String output;
          serializeJson(response, output);
          webServer.send(200, "application/json", output);
          return;
        }
      }
    }
    
    webServer.send(400, "application/json", "{\"error\":\"Invalid request\"}");
  });
  
  // POST /stop - Detener sistema
  webServer.on("/stop", HTTP_POST, []() {
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    
    classifier.stopSystem();
    
    StaticJsonDocument<200> response;
    response["action"] = "stop";
    response["status"] = "stopped";
    response["timestamp"] = millis();
    
    String output;
    serializeJson(response, output);
    webServer.send(200, "application/json", output);
  });
  
  // GET /sensors - Datos de sensores
  webServer.on("/sensors", HTTP_GET, []() {
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    
    StaticJsonDocument<200> response;
    response["device"] = "upcyclepro";
    response["timestamp"] = millis();
    response["pir1"] = digitalRead(26);
    response["pir2"] = digitalRead(27);
    response["pir3"] = digitalRead(14);
    response["motor"] = classifier.getState() == WAITING ? true : false;
    
    String output;
    serializeJson(response, output);
    webServer.send(200, "application/json", output);
  });
  
  webServer.begin();
  Serial.println("🌐 Servidor web iniciado en puerto 80");
}

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("🔌 WebSocket [%u] desconectado\n", num);
      break;
      
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("🔌 WebSocket [%u] conectado desde %d.%d.%d.%d\n", 
                    num, ip[0], ip[1], ip[2], ip[3]);
      // Enviar estado actual al conectar
      sendStatusUpdate();
      break;
    }
    
    case WStype_TEXT: {
      String message = String((char*)payload);
      Serial.printf("📨 WebSocket [%u]: %s\n", num, message.c_str());
      
      // Parsear comando JSON
      StaticJsonDocument<200> doc;
      if (deserializeJson(doc, message) == DeserializationError::Ok) {
        handleCommand(doc);
      }
      break;
    }
    
    default:
      break;
  }
}

/*
===============================================================================
                                API DOCUMENTATION
===============================================================================

🔗 ENDPOINTS REST:

GET /status
- Respuesta: Estado actual del sistema
- Formato: {"device":"upcyclepro", "state":"idle", "mode":0, "material":"none"}

POST /classify
- Body: {"material": 1-3}  // 1=vidrio, 2=plastico, 3=metal
- Respuesta: Confirmación de inicio

POST /stop  
- Respuesta: Confirmación de parada

GET /sensors
- Respuesta: Estado de sensores PIR y motor

📡 TÓPICOS MQTT:

upcyclepro/command (INPUT):
{"action": "classify", "material": 1}
{"action": "stop"}
{"action": "get_status"}

upcyclepro/status (OUTPUT):
{"device":"upcyclepro", "state":"waiting", "mode":1, "material":"vidrio"}

upcyclepro/sensor (OUTPUT):
{"pir1":false, "pir2":true, "pir3":false, "motor":true}

upcyclepro/material (OUTPUT):
{"material":"plastico", "processed_at":"timestamp", "servo_used":2}

🔌 WEBSOCKET (Puerto 81):
- Conexión: ws://IP:81
- Envía: Mismo formato que MQTT commands
- Recibe: Estados y sensores en tiempo real

===============================================================================
                                NODE-RED FLOWS
===============================================================================

FLOW BÁSICO DE CONTROL:
[inject] → [function] → [mqtt out: upcyclepro/command]
                    ↓
              {"action": "classify", "material": 1}

FLOW DE MONITOREO:
[mqtt in: upcyclepro/status] → [debug]
[mqtt in: upcyclepro/sensor] → [chart/gauge]

FLOW HTTP:
[http request] → [http response]
     ↓              ↑
   POST /classify   200 OK

===============================================================================
*/