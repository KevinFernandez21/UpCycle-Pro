/*
  CAMERA WEB SERVER - INTEGRACIÓN NODE-RED
  =======================================
  
  ESP32-CAM adaptado para Node-RED:
  - Stream de video compatible con Node-RED
  - API REST para captura de imágenes
  - MQTT para notificaciones
  - WebSocket para streaming en tiempo real
  - Endpoints optimizados para análisis de materiales
  
  Funcionalidades Node-RED:
  - GET /stream - Video stream MJPEG
  - GET /capture - Captura imagen JPEG
  - POST /analyze - Envía imagen para análisis IA
  - WebSocket /ws - Stream tiempo real
  - MQTT: camera/image, camera/status
*/

#include "esp_camera.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <base64.h>
#include "board_config.h"

// Configuración WiFi
const char* ssid = "NETLIFE-SANCHEZ";
const char* password = "kd200421";

// Configuración MQTT
const char* mqtt_server = "localhost";  // IP de Node-RED
const int mqtt_port = 1883;
const char* mqtt_client_id = "upcyclepro_camera";

// Tópicos MQTT
const char* topic_image = "camera/image";
const char* topic_status = "camera/status";
const char* topic_command = "camera/command";

// Instancias
WiFiClient espClient;
PubSubClient mqtt(espClient);
WebServer webServer(80);
WebSocketsServer webSocket(81);

// Variables de control
bool streaming_active = false;
unsigned long last_capture = 0;
unsigned long last_status = 0;
const unsigned long STATUS_INTERVAL = 10000;  // 10 segundos

// Headers HTTP para CORS
void setCORSHeaders() {
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  webServer.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("🎥 Iniciando Camera Server Node-RED");

  // Configurar cámara
  if (!initCamera()) {
    Serial.println("❌ Error inicializando cámara");
    return;
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
  
  Serial.println("🚀 Camera Server Node-RED listo");
  Serial.printf("📹 Stream: http://%s/stream\n", WiFi.localIP().toString().c_str());
  Serial.printf("📸 Capture: http://%s/capture\n", WiFi.localIP().toString().c_str());
  Serial.printf("🔌 WebSocket: ws://%s:81\n", WiFi.localIP().toString().c_str());
  
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
  
  // Enviar estado periódicamente
  if (millis() - last_status > STATUS_INTERVAL) {
    sendStatusUpdate();
    last_status = millis();
  }
  
  delay(100);
}

bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_SVGA;  // Optimizado para análisis
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (psramFound()) {
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("❌ Camera init failed: 0x%x", err);
    return false;
  }

  // Configurar sensor
  sensor_t* s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }

  return true;
}

void setupWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Conectando WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.printf("📶 WiFi conectado - IP: %s\n", WiFi.localIP().toString().c_str());
}

void reconnectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("🔄 Conectando MQTT...");
    
    if (mqtt.connect(mqtt_client_id)) {
      Serial.println(" ✅ Conectado");
      mqtt.subscribe(topic_command);
      sendStatusUpdate();
    } else {
      Serial.printf(" ❌ Error: %d\n", mqtt.state());
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
  
  StaticJsonDocument<200> doc;
  if (deserializeJson(doc, message) == DeserializationError::Ok) {
    handleCommand(doc);
  }
}

void handleCommand(JsonDocument& cmd) {
  if (cmd.containsKey("action")) {
    String action = cmd["action"];
    
    if (action == "capture") {
      captureAndSendImage();
    } else if (action == "start_stream") {
      streaming_active = true;
      sendStatusUpdate();
    } else if (action == "stop_stream") {
      streaming_active = false;
      sendStatusUpdate();
    } else if (action == "get_status") {
      sendStatusUpdate();
    }
  }
}

void setupWebServer() {
  // Manejar CORS
  webServer.onNotFound([]() {
    setCORSHeaders();
    if (webServer.method() == HTTP_OPTIONS) {
      webServer.send(200);
      return;
    }
    webServer.send(404, "text/plain", "Not Found");
  });

  // GET / - Página de información
  webServer.on("/", HTTP_GET, []() {
    setCORSHeaders();
    String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>UpCyclePro Camera - Node-RED</title>
    <meta charset="utf-8">
    <style>
        body { font-family: Arial; margin: 40px; background: #f5f5f5; }
        .container { max-width: 800px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; }
        .status { background: #e8f5e8; padding: 15px; border-radius: 5px; margin: 20px 0; }
        .endpoint { background: #f0f0f0; padding: 10px; margin: 10px 0; border-radius: 5px; font-family: monospace; }
        img { max-width: 100%; height: auto; border: 2px solid #ddd; border-radius: 5px; }
        button { background: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 5px; margin: 5px; cursor: pointer; }
        button:hover { background: #0056b3; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎥 UpCyclePro Camera Server</h1>
        <p>ESP32-CAM integrado con Node-RED para análisis de materiales reciclables</p>
        
        <div class="status">
            <h3>📊 Estado del Sistema</h3>
            <p><strong>IP:</strong> )" + WiFi.localIP().toString() + R"(</p>
            <p><strong>WiFi:</strong> )" + String(WiFi.RSSI()) + R"( dBm</p>
            <p><strong>Memoria:</strong> )" + String(ESP.getFreeHeap()) + R"( bytes</p>
        </div>

        <h3>🔗 Endpoints Node-RED</h3>
        <div class="endpoint">GET /stream - Video stream MJPEG</div>
        <div class="endpoint">GET /capture - Captura imagen JPEG</div>
        <div class="endpoint">POST /analyze - Envía imagen para análisis</div>
        <div class="endpoint">GET /status - Estado de la cámara</div>
        <div class="endpoint">WebSocket ws://)" + WiFi.localIP().toString() + R"(:81</div>

        <h3>📸 Captura en Vivo</h3>
        <button onclick="captureImage()">📷 Capturar Imagen</button>
        <button onclick="toggleStream()">🎬 Toggle Stream</button>
        <br><br>
        <img id="capture" src="/capture" alt="Captura de cámara">

        <h3>📡 MQTT Topics</h3>
        <div class="endpoint">camera/image - Imágenes en base64</div>
        <div class="endpoint">camera/status - Estado del sistema</div>
        <div class="endpoint">camera/command - Comandos de control</div>
    </div>

    <script>
        function captureImage() {
            document.getElementById('capture').src = '/capture?' + new Date().getTime();
        }
        
        function toggleStream() {
            // Implementar toggle de stream
            fetch('/toggle_stream', {method: 'POST'});
        }
        
        // Auto-refresh cada 5 segundos
        setInterval(captureImage, 5000);
    </script>
</body>
</html>
)";
    webServer.send(200, "text/html", html);
  });

  // GET /stream - MJPEG Stream
  webServer.on("/stream", HTTP_GET, handleStream);

  // GET /capture - Captura única
  webServer.on("/capture", HTTP_GET, []() {
    setCORSHeaders();
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      webServer.send(500, "text/plain", "Camera capture failed");
      return;
    }

    webServer.send(200, "image/jpeg", (const char*)fb->buf, fb->len);
    esp_camera_fb_return(fb);
    
    last_capture = millis();
  });

  // POST /analyze - Enviar imagen para análisis
  webServer.on("/analyze", HTTP_POST, []() {
    setCORSHeaders();
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      webServer.send(500, "application/json", "{\"error\":\"Camera capture failed\"}");
      return;
    }

    // Convertir a base64 para envío por MQTT
    String base64Image = base64::encode(fb->buf, fb->len);
    
    StaticJsonDocument<1000> response;
    response["action"] = "analyze";
    response["timestamp"] = millis();
    response["image_size"] = fb->len;
    response["format"] = "jpeg";
    response["base64"] = base64Image.substring(0, 100) + "..."; // Truncado para respuesta
    
    // Enviar imagen completa por MQTT
    StaticJsonDocument<2000> mqttMsg;
    mqttMsg["action"] = "image_analysis";
    mqttMsg["timestamp"] = millis();
    mqttMsg["image"] = base64Image;
    mqttMsg["width"] = fb->width;
    mqttMsg["height"] = fb->height;
    
    String mqttOutput;
    serializeJson(mqttMsg, mqttOutput);
    mqtt.publish(topic_image, mqttOutput.c_str());

    esp_camera_fb_return(fb);
    
    String output;
    serializeJson(response, output);
    webServer.send(200, "application/json", output);
  });

  // GET /status - Estado de la cámara
  webServer.on("/status", HTTP_GET, []() {
    setCORSHeaders();
    
    StaticJsonDocument<300> status;
    status["device"] = "upcyclepro_camera";
    status["timestamp"] = millis();
    status["streaming"] = streaming_active;
    status["last_capture"] = last_capture;
    status["wifi_rssi"] = WiFi.RSSI();
    status["free_heap"] = ESP.getFreeHeap();
    status["ip"] = WiFi.localIP().toString();
    
    String output;
    serializeJson(status, output);
    webServer.send(200, "application/json", output);
  });

  // POST /toggle_stream
  webServer.on("/toggle_stream", HTTP_POST, []() {
    setCORSHeaders();
    streaming_active = !streaming_active;
    
    StaticJsonDocument<200> response;
    response["action"] = "toggle_stream";
    response["streaming"] = streaming_active;
    response["timestamp"] = millis();
    
    String output;
    serializeJson(response, output);
    webServer.send(200, "application/json", output);
  });

  webServer.begin();
  Serial.println("🌐 Servidor web iniciado en puerto 80");
}

void handleStream() {
  setCORSHeaders();
  
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  webServer.sendContent(response);

  streaming_active = true;
  
  while (streaming_active && webServer.client().connected()) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("❌ Error capturando frame");
      break;
    }

    String frame = "--frame\r\n";
    frame += "Content-Type: image/jpeg\r\n";
    frame += "Content-Length: " + String(fb->len) + "\r\n\r\n";
    
    webServer.sendContent(frame);
    webServer.sendContent((const char*)fb->buf, fb->len);
    webServer.sendContent("\r\n");
    
    esp_camera_fb_return(fb);
    
    if (!webServer.client().connected()) {
      break;
    }
    
    delay(100); // ~10 FPS
  }
  
  streaming_active = false;
}

void captureAndSendImage() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("❌ Error capturando imagen");
    return;
  }

  String base64Image = base64::encode(fb->buf, fb->len);
  
  StaticJsonDocument<2000> mqttMsg;
  mqttMsg["action"] = "image_capture";
  mqttMsg["timestamp"] = millis();
  mqttMsg["image"] = base64Image;
  mqttMsg["size"] = fb->len;
  
  String output;
  serializeJson(mqttMsg, output);
  mqtt.publish(topic_image, output.c_str());
  
  esp_camera_fb_return(fb);
  last_capture = millis();
  
  Serial.println("📸 Imagen enviada por MQTT");
}

void sendStatusUpdate() {
  StaticJsonDocument<300> status;
  status["device"] = "upcyclepro_camera";
  status["timestamp"] = millis();
  status["streaming"] = streaming_active;
  status["last_capture"] = last_capture;
  status["wifi_rssi"] = WiFi.RSSI();
  status["free_heap"] = ESP.getFreeHeap();
  status["uptime"] = millis() / 1000;
  
  String output;
  serializeJson(status, output);
  mqtt.publish(topic_status, output.c_str());
  
  // También enviar por WebSocket
  webSocket.broadcastTXT(output);
}

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("🔌 WebSocket [%u] desconectado\n", num);
      break;
      
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("🔌 WebSocket [%u] conectado desde %d.%d.%d.%d\n", 
                    num, ip[0], ip[1], ip[2], ip[3]);
      sendStatusUpdate();
      break;
    }
    
    case WStype_TEXT: {
      String message = String((char*)payload);
      Serial.printf("📨 WebSocket [%u]: %s\n", num, message.c_str());
      
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
                            INTEGRACIÓN NODE-RED
===============================================================================

🎯 NODOS RECOMENDADOS:

1. MQTT IN/OUT - Para comunicación
2. HTTP REQUEST - Para llamadas REST
3. WebSocket - Para streaming tiempo real
4. Function - Para procesamiento JSON
5. Dashboard - Para visualización
6. File - Para guardar imágenes
7. Computer Vision - Para análisis IA

📋 FLOWS EJEMPLO:

FLOW CAPTURA:
[inject] → [http request: /capture] → [base64 decode] → [file out]

FLOW ANÁLISIS:
[mqtt in: camera/image] → [function: parse] → [AI analysis] → [classification]

FLOW MONITORING:
[mqtt in: camera/status] → [gauge/chart]

FLOW STREAM:
[http request: /stream] → [dashboard template]

🔧 CONFIGURACIÓN MQTT:

Broker: localhost:1883
Topics:
- camera/image (OUT) - Imágenes base64
- camera/status (OUT) - Estado sistema  
- camera/command (IN) - Comandos control

📡 API ENDPOINTS:

GET /stream - Video MJPEG stream
GET /capture - Imagen JPEG única
POST /analyze - Captura + análisis
GET /status - Estado cámara
WebSocket :81 - Datos tiempo real

===============================================================================
*/