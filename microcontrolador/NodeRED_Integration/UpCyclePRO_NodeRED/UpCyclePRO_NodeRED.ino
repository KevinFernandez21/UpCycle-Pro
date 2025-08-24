/*
  CLASIFICADORA DE MATERIALES - INTEGRACIÓN BACKEND API
  ===================================================
  
  Sistema UpCyclePRO con comunicación directa al backend FastAPI:
  - Comunicación WiFi + HTTP API
  - Auto-registro con backend
  - Envío periódico de datos de sensores
  - Recepción de comandos del backend
  - JSON para todos los mensajes
  
  Endpoints Backend:
  - POST /api/esp32-control/sensors - Enviar datos de sensores
  - POST /api/classification/result - Enviar resultado de clasificación
  - GET /status - Estado local del dispositivo
  - POST /classify - Comando de clasificación local
  - GET /sensors - Lectura de sensores local
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include "SimpleClassifier.h"

// Configuración WiFi
const char* ssid = "hnaranja21";
const char* password = "hola12345";

// Configuración Backend API
const char* api_server = "192.168.1.100";  // IP del servidor FastAPI
const int api_port = 8000;
String device_id = "esp32_main_001";
String device_type = "esp32-control";

// Servidor web local para comunicación directa
WebServer webServer(80);
SimpleClassifier classifier;

// Variables de estado y timing
unsigned long lastSensorSend = 0;
unsigned long lastStatusCheck = 0;
unsigned long lastBackendPing = 0;
const unsigned long SENSOR_INTERVAL = 5000;    // 5 segundos
const unsigned long STATUS_INTERVAL = 30000;   // 30 segundos  
const unsigned long BACKEND_PING_INTERVAL = 60000; // 1 minuto

// Estado de conexión con backend
bool backendConnected = false;
unsigned long backendLastSeen = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("========================================");
  Serial.println("   CLASIFICADORA ESP32 - API BACKEND");
  Serial.println("========================================");
  
  // Inicializar clasificador
  if (!classifier.begin()) {
    Serial.println("❌ Error inicializando sistema clasificador");
    while(1) delay(1000);
  }
  
  // Conectar WiFi
  setupWiFi();
  
  // Configurar servidor web local
  setupWebServer();
  
  Serial.println("🚀 Sistema API Backend listo");
  Serial.println("🔗 Backend API: http://" + String(api_server) + ":" + String(api_port));
  Serial.println("🌐 Servidor local: http://" + WiFi.localIP().toString() + ":80");
  
  // Registrar dispositivo con el backend
  registerWithBackend();
  
  // Enviar estado inicial
  sendSensorDataToBackend();
}

void loop() {
  // Manejar servidor web local
  webServer.handleClient();
  
  // Actualizar clasificador
  classifier.update();
  
  // Enviar datos de sensores al backend periódicamente
  if (millis() - lastSensorSend > SENSOR_INTERVAL) {
    sendSensorDataToBackend();
    lastSensorSend = millis();
  }
  
  // Ping al backend para mantener conexión
  if (millis() - lastBackendPing > BACKEND_PING_INTERVAL) {
    pingBackend();
    lastBackendPing = millis();
  }
  
  // Verificar si backend está disponible
  if (millis() - backendLastSeen > STATUS_INTERVAL * 2) {
    backendConnected = false;
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
  Serial.print("📶 Señal WiFi: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

// =================== COMUNICACIÓN BACKEND API ===================

void registerWithBackend() {
  Serial.println("📡 Registrando dispositivo con backend API...");
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi no conectado para registro");
    return;
  }
  
  HTTPClient http;
  String url = "http://" + String(api_server) + ":" + String(api_port) + "/api/esp32-cam/register";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  String postData = "device_id=" + device_id + "&ip_address=" + WiFi.localIP().toString();
  
  int httpResponseCode = http.POST(postData);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    if (httpResponseCode == 200 || response.indexOf("success") > 0) {
      Serial.println("✅ Dispositivo registrado exitosamente en backend");
      backendConnected = true;
      backendLastSeen = millis();
    } else {
      Serial.println("⚠️ Error en registro, código: " + String(httpResponseCode));
      Serial.println("Respuesta: " + response);
    }
  } else {
    Serial.println("❌ Error conectando al backend para registro: " + String(httpResponseCode));
  }
  
  http.end();
}

void sendSensorDataToBackend() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  HTTPClient http;
  String url = "http://" + String(api_server) + ":" + String(api_port) + "/api/esp32-control/sensors";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  // Crear JSON con datos de sensores
  StaticJsonDocument<300> sensorData;
  sensorData["device_id"] = device_id;
  sensorData["timestamp"] = millis();
  sensorData["pir1"] = digitalRead(26);  // PIR Vidrio
  sensorData["pir2"] = digitalRead(27);  // PIR Plástico
  sensorData["pir3"] = digitalRead(14);  // PIR Metal
  sensorData["conveyor_active"] = (classifier.getState() == WAITING);
  sensorData["weight1"] = 0.0; // Placeholder para sensores de peso
  sensorData["weight2"] = 0.0;
  sensorData["weight3"] = 0.0;
  
  // Información adicional del dispositivo
  JsonObject servo_positions = sensorData.createNestedObject("servo_positions");
  servo_positions["servo1"] = 90; // Posiciones actuales de servos
  servo_positions["servo2"] = 90;
  servo_positions["servo3"] = 90;
  
  String jsonString;
  serializeJson(sensorData, jsonString);
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    backendConnected = true;
    backendLastSeen = millis();
    
    // Procesar comandos pendientes del backend
    processBackendResponse(response);
    
    if (httpResponseCode != 200) {
      Serial.println("⚠️ Respuesta backend: " + String(httpResponseCode));
    }
  } else {
    Serial.println("❌ Error enviando sensores al backend: " + String(httpResponseCode));
    backendConnected = false;
  }
  
  http.end();
}

void processBackendResponse(String response) {
  StaticJsonDocument<1000> doc;
  
  if (deserializeJson(doc, response) != DeserializationError::Ok) {
    return; // JSON inválido
  }
  
  // Verificar comandos pendientes
  if (doc.containsKey("pending_commands")) {
    JsonArray commands = doc["pending_commands"];
    
    for (JsonObject cmd : commands) {
      String command = cmd["command"];
      JsonObject parameters = cmd["parameters"];
      
      Serial.println("🤖 Ejecutando comando del backend: " + command);
      
      if (command == "move_servo") {
        String material = parameters["material"];
        int position = parameters["position"];
        
        executeServoCommand(material, position);
        
      } else if (command == "classify") {
        String material = parameters["material"] | "auto";
        
        executeClassificationCommand(material);
        
      } else if (command == "stop") {
        executeStopCommand();
        
      } else if (command == "start_conveyor") {
        classifier.startContinuousOperation();
        Serial.println("🔄 Banda transportadora iniciada por comando backend");
        
      } else {
        Serial.println("⚠️ Comando desconocido: " + command);
      }
    }
  }
}

void executeServoCommand(String material, int position) {
  Serial.println("🔧 Ejecutando comando servo - Material: " + material + ", Posición: " + String(position));
  
  // Configurar modo del clasificador
  classifier.setMode(material);
  
  // Simular activación del servo correspondiente
  if (material == "glass" || material == "vidrio") {
    // Activar servo 1 para vidrio
    Serial.println("🔹 Activando servo 1 (Vidrio) -> " + String(position) + "°");
  } else if (material == "plastic" || material == "plastico") {
    // Activar servo 2 para plástico
    Serial.println("🔸 Activando servo 2 (Plástico) -> " + String(position) + "°");
  } else if (material == "metal") {
    // Activar servo 3 para metal
    Serial.println("🔶 Activando servo 3 (Metal) -> " + String(position) + "°");
  }
  
  // Simular tiempo de activación del servo
  delay(2000);
  
  // Enviar confirmación al backend
  sendClassificationResultToBackend(material, 0.95, position);
}

void executeClassificationCommand(String material) {
  Serial.println("🧠 Iniciando proceso de clasificación: " + material);
  
  if (material != "auto") {
    classifier.setMode(material);
  }
  
  classifier.startContinuousOperation();
  
  // Simular proceso de clasificación
  delay(1000);
  
  // Enviar resultado simulado
  float confidence = 0.85 + (random(0, 15) / 100.0); // Confianza simulada 85-100%
  int servoPosition = getServoPositionForMaterial(material);
  
  sendClassificationResultToBackend(material, confidence, servoPosition);
}

void executeStopCommand() {
  Serial.println("🛑 Deteniendo sistema por comando backend");
  classifier.stopSystem();
}

int getServoPositionForMaterial(String material) {
  if (material == "glass" || material == "vidrio") return 45;
  if (material == "plastic" || material == "plastico") return 90;
  if (material == "metal") return 135;
  return 90; // Posición por defecto
}

void sendClassificationResultToBackend(String material, float confidence, int servoPosition) {
  HTTPClient http;
  String url = "http://" + String(api_server) + ":" + String(api_port) + "/api/classification/result";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  StaticJsonDocument<300> result;
  result["device_id"] = device_id;
  result["material"] = material;
  result["confidence"] = confidence;
  result["servo_position"] = servoPosition;
  result["processing_time"] = 2.5; // Tiempo simulado en segundos
  result["timestamp"] = millis();
  
  String jsonString;
  serializeJson(result, jsonString);
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode == 200) {
    Serial.println("📊 Resultado enviado al backend: " + material + " (" + String(confidence, 2) + ")");
  } else {
    Serial.println("❌ Error enviando resultado al backend: " + String(httpResponseCode));
  }
  
  http.end();
}

void pingBackend() {
  HTTPClient http;
  String url = "http://" + String(api_server) + ":" + String(api_port) + "/health";
  
  http.begin(url);
  http.setTimeout(5000); // 5 segundos timeout
  
  int httpResponseCode = http.GET();
  
  if (httpResponseCode == 200) {
    backendConnected = true;
    backendLastSeen = millis();
  } else {
    backendConnected = false;
    Serial.println("⚠️ Backend no responde al ping: " + String(httpResponseCode));
  }
  
  http.end();
}

// =================== SERVIDOR WEB LOCAL ===================

void setupWebServer() {
  // Configurar CORS para todas las rutas
  webServer.onNotFound([]() {
    setCORSHeaders();
    
    if (webServer.method() == HTTP_OPTIONS) {
      webServer.send(200);
      return;
    }
    webServer.send(404, "application/json", "{\"error\":\"Endpoint no encontrado\"}");
  });
  
  // GET /status - Estado actual del dispositivo
  webServer.on("/status", HTTP_GET, []() {
    setCORSHeaders();
    
    StaticJsonDocument<400> response;
    response["device_id"] = device_id;
    response["device_type"] = device_type;
    response["timestamp"] = millis();
    response["state"] = getStateString(classifier.getState());
    response["mode"] = classifier.getMode();
    response["ip"] = WiFi.localIP().toString();
    response["wifi_rssi"] = WiFi.RSSI();
    response["backend_connected"] = backendConnected;
    response["uptime"] = millis() / 1000;
    response["free_heap"] = ESP.getFreeHeap();
    
    String output;
    serializeJson(response, output);
    webServer.send(200, "application/json", output);
  });
  
  // POST /classify - Comando de clasificación local
  webServer.on("/classify", HTTP_POST, []() {
    setCORSHeaders();
    
    if (!webServer.hasArg("plain")) {
      webServer.send(400, "application/json", "{\"error\":\"Body JSON requerido\"}");
      return;
    }
    
    StaticJsonDocument<200> doc;
    if (deserializeJson(doc, webServer.arg("plain")) != DeserializationError::Ok) {
      webServer.send(400, "application/json", "{\"error\":\"JSON inválido\"}");
      return;
    }
    
    if (!doc.containsKey("material")) {
      webServer.send(400, "application/json", "{\"error\":\"Campo 'material' requerido\"}");
      return;
    }
    
    String material = doc["material"];
    executeClassificationCommand(material);
    
    StaticJsonDocument<200> response;
    response["status"] = "success";
    response["message"] = "Clasificación iniciada";
    response["material"] = material;
    response["timestamp"] = millis();
    
    String output;
    serializeJson(response, output);
    webServer.send(200, "application/json", output);
  });
  
  // GET /sensors - Lectura de sensores actual
  webServer.on("/sensors", HTTP_GET, []() {
    setCORSHeaders();
    
    StaticJsonDocument<300> response;
    response["device_id"] = device_id;
    response["timestamp"] = millis();
    response["pir1"] = digitalRead(26);
    response["pir2"] = digitalRead(27);
    response["pir3"] = digitalRead(14);
    response["conveyor_active"] = (classifier.getState() == WAITING);
    response["weight1"] = 0.0;
    response["weight2"] = 0.0;
    response["weight3"] = 0.0;
    
    String output;
    serializeJson(response, output);
    webServer.send(200, "application/json", output);
  });
  
  // POST /stop - Detener sistema
  webServer.on("/stop", HTTP_POST, []() {
    setCORSHeaders();
    
    executeStopCommand();
    
    StaticJsonDocument<200> response;
    response["status"] = "success";
    response["message"] = "Sistema detenido";
    response["timestamp"] = millis();
    
    String output;
    serializeJson(response, output);
    webServer.send(200, "application/json", output);
  });
  
  // POST /start - Iniciar sistema
  webServer.on("/start", HTTP_POST, []() {
    setCORSHeaders();
    
    classifier.startContinuousOperation();
    
    StaticJsonDocument<200> response;
    response["status"] = "success";
    response["message"] = "Sistema iniciado";
    response["timestamp"] = millis();
    
    String output;
    serializeJson(response, output);
    webServer.send(200, "application/json", output);
  });
  
  webServer.begin();
  Serial.println("🌐 Servidor web local iniciado en puerto 80");
}

void setCORSHeaders() {
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  webServer.sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

String getStateString(SystemState state) {
  switch(state) {
    case IDLE: return "idle";
    case BELT_RUNNING: return "belt_running";
    case CAMERA_DETECTING: return "camera_detecting";
    case WAITING_FOR_PIR: return "waiting_for_pir";
    case WAITING: return "waiting";
    case PROCESSING: return "processing";
    default: return "unknown";
  }
}

/*
===============================================================================
                            API LOCAL DOCUMENTATION
===============================================================================

🔗 ENDPOINTS DISPONIBLES:

GET /status
- Respuesta: Estado completo del dispositivo
- Formato: {"device_id":"esp32_main_001", "state":"idle", "backend_connected":true}

POST /classify
- Body: {"material": "glass|plastic|metal|auto"}
- Respuesta: Confirmación de inicio de clasificación

GET /sensors  
- Respuesta: Lectura actual de todos los sensores

POST /stop
- Respuesta: Confirmación de parada del sistema

POST /start
- Respuesta: Confirmación de inicio del sistema

📡 COMUNICACIÓN CON BACKEND:

El dispositivo automáticamente:
- Se registra con el backend al iniciar
- Envía datos de sensores cada 5 segundos
- Procesa comandos del backend
- Envía resultados de clasificación
- Mantiene conexión con pings cada minuto

===============================================================================
*/