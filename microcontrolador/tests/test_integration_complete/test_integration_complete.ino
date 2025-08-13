/*
  TEST INTEGRACIÓN COMPLETA - UpCycle Pro
  ========================================
  
  Este sketch prueba todos los componentes del sistema integrados,
  simulando el funcionamiento completo del clasificador de materiales.
  
  Componentes incluidos:
  - 3 Sensores PIR (GPIO26, 27, 14)
  - 1 Motor con transistor 2N222 (GPIO25)
  - 1 PCA9685 con 3 servos (I2C)
  - 1 Display OLED SSD1306 (I2C)
  
  Funcionamiento:
  - Simula ciclo completo de clasificación
  - Prueba coordinada de todos los componentes
  - Manejo de errores y diagnóstico
  - Interface completa por comandos seriales
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===============================================================================
// DEFINICIONES Y CONFIGURACIÓN
// ===============================================================================

// Pines PIR
#define PIR1_PIN 26  // Sensor VIDRIO
#define PIR2_PIN 27  // Sensor PLASTICO
#define PIR3_PIN 14  // Sensor METAL

// Pin Motor
#define MOTOR_PIN 25 // Control del motor vía 2N222

// LEDs indicadores
#define LED_PIN 2    // LED onboard

// Configuración OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Configuración Servos
#define SERVO_VIDRIO 0    // Canal 0 PCA9685
#define SERVO_PLASTICO 1  // Canal 1 PCA9685
#define SERVO_METAL 2     // Canal 2 PCA9685
#define SERVO_MIN 150     // Posición reposo
#define SERVO_MAX 600     // Posición empuje

// Configuración PWM Motor
const int pwm_frequency = 1000;
const int pwm_channel = 0;
const int pwm_resolution = 8;

// ===============================================================================
// OBJETOS GLOBALES
// ===============================================================================

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ===============================================================================
// VARIABLES DE ESTADO
// ===============================================================================

// Sistema general
enum SystemState {
  SYSTEM_IDLE,
  SYSTEM_WAITING,
  SYSTEM_PROCESSING,
  SYSTEM_ERROR
};

SystemState current_state = SYSTEM_IDLE;
int current_material = 0; // 0=NONE, 1=VIDRIO, 2=PLASTICO, 3=METAL
bool motor_running = false;
int motor_speed = 0;

// Sensores PIR
bool pir1_state = false, pir2_state = false, pir3_state = false;
bool pir1_last = false, pir2_last = false, pir3_last = false;
unsigned long pir1_count = 0, pir2_count = 0, pir3_count = 0;

// Timing
unsigned long state_start_time = 0;
unsigned long last_display_update = 0;
unsigned long last_status_report = 0;
const unsigned long TIMEOUT_WAITING = 30000; // 30 segundos
const unsigned long DISPLAY_UPDATE_INTERVAL = 500; // 0.5 segundos
const unsigned long STATUS_REPORT_INTERVAL = 5000; // 5 segundos

// Contadores estadísticos
unsigned long total_processed = 0;
unsigned long vidrio_count = 0;
unsigned long plastico_count = 0;
unsigned long metal_count = 0;
unsigned long error_count = 0;

// Estado de componentes
bool oled_ok = false;
bool pca9685_ok = false;
bool system_initialized = false;

// ===============================================================================
// SETUP
// ===============================================================================

void setup() {
  Serial.begin(115200);
  
  Serial.println();
  Serial.println("🚀 TEST INTEGRACIÓN COMPLETA - UpCycle Pro");
  Serial.println("==========================================");
  Serial.println();
  
  // Inicializar hardware
  if (!initializeHardware()) {
    Serial.println("❌ ERROR CRÍTICO: Fallo en inicialización");
    while(1) {
      blinkError();
      delay(1000);
    }
  }
  
  // Estado inicial
  current_state = SYSTEM_IDLE;
  updateDisplay();
  
  Serial.println("✅ Sistema completamente inicializado");
  Serial.println();
  printHelp();
  
  system_initialized = true;
}

// ===============================================================================
// LOOP PRINCIPAL
// ===============================================================================

void loop() {
  // Procesar comandos seriales
  if (Serial.available()) {
    processSerialCommand();
  }
  
  // Leer sensores
  readSensors();
  
  // Ejecutar máquina de estados
  runStateMachine();
  
  // Actualizar display periódicamente
  if (millis() - last_display_update > DISPLAY_UPDATE_INTERVAL) {
    updateDisplay();
    last_display_update = millis();
  }
  
  // Reporte de estado periódico
  if (millis() - last_status_report > STATUS_REPORT_INTERVAL) {
    printStatusReport();
    last_status_report = millis();
  }
  
  // Actualizar LED indicador
  updateStatusLED();
  
  delay(50);
}

// ===============================================================================
// INICIALIZACIÓN DE HARDWARE
// ===============================================================================

bool initializeHardware() {
  Serial.println("🔄 Inicializando hardware...");
  
  // Configurar pines
  pinMode(PIR1_PIN, INPUT);
  pinMode(PIR2_PIN, INPUT);
  pinMode(PIR3_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Configurar PWM para motor
  ledcSetup(pwm_channel, pwm_frequency, pwm_resolution);
  ledcAttachPin(MOTOR_PIN, pwm_channel);
  setMotorSpeed(0);
  
  // Inicializar I2C
  Wire.begin();
  Wire.setClock(100000); // 100kHz para estabilidad
  
  // Inicializar OLED
  Serial.print("📺 Inicializando OLED... ");
  if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    oled_ok = true;
    Serial.println("✅");
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("UpCycle Pro");
    display.println("Inicializando...");
    display.display();
  } else {
    Serial.println("❌");
    oled_ok = false;
  }
  
  // Inicializar PCA9685
  Serial.print("🎛️ Inicializando PCA9685... ");
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(50);
  
  // Verificar comunicación PCA9685
  Wire.beginTransmission(0x40);
  if (Wire.endTransmission() == 0) {
    pca9685_ok = true;
    Serial.println("✅");
    
    // Mover servos a posición inicial
    pwm.setPWM(SERVO_VIDRIO, 0, SERVO_MIN);
    pwm.setPWM(SERVO_PLASTICO, 0, SERVO_MIN);
    pwm.setPWM(SERVO_METAL, 0, SERVO_MIN);
  } else {
    Serial.println("❌");
    pca9685_ok = false;
  }
  
  delay(2000); // Tiempo para estabilización
  
  // Verificar componentes críticos
  if (!oled_ok || !pca9685_ok) {
    Serial.println();
    Serial.println("⚠️ ADVERTENCIA: Algunos componentes no están disponibles");
    if (!oled_ok) Serial.println("   - OLED no detectado");
    if (!pca9685_ok) Serial.println("   - PCA9685 no detectado");
    Serial.println("   El sistema funcionará con funcionalidad limitada");
    Serial.println();
  }
  
  return true; // Siempre continuar, incluso con componentes faltantes
}

// ===============================================================================
// MÁQUINA DE ESTADOS
// ===============================================================================

void runStateMachine() {
  switch (current_state) {
    case SYSTEM_IDLE:
      // Sistema en reposo, esperando comando de material
      break;
      
    case SYSTEM_WAITING:
      handleWaitingState();
      break;
      
    case SYSTEM_PROCESSING:
      handleProcessingState();
      break;
      
    case SYSTEM_ERROR:
      handleErrorState();
      break;
  }
}

void handleWaitingState() {
  // Verificar timeout
  if (millis() - state_start_time > TIMEOUT_WAITING) {
    Serial.println("⏰ TIMEOUT: No se detectó material en 30 segundos");
    stopSystem();
    return;
  }
  
  // Verificar detección según material activo
  bool detected = false;
  
  switch (current_material) {
    case 1: // VIDRIO
      if (pir1_state && !pir1_last) {
        Serial.println("🔵 VIDRIO detectado por PIR1!");
        detected = true;
      }
      break;
    case 2: // PLASTICO
      if (pir2_state && !pir2_last) {
        Serial.println("🟠 PLASTICO detectado por PIR2!");
        detected = true;
      }
      break;
    case 3: // METAL
      if (pir3_state && !pir3_last) {
        Serial.println("⚫ METAL detectado por PIR3!");
        detected = true;
      }
      break;
  }
  
  if (detected) {
    startProcessing();
  }
}

void handleProcessingState() {
  // El procesamiento es automático, simplemente esperar a que termine
  // (El procesamiento se maneja en startProcessing())
}

void handleErrorState() {
  // En estado de error, solo se puede salir con reset manual
  blinkError();
}

// ===============================================================================
// CONTROL DEL SISTEMA
// ===============================================================================

void startMaterial(int material) {
  if (current_state != SYSTEM_IDLE) {
    Serial.println("❌ Sistema ocupado. Use 'stop' primero.");
    return;
  }
  
  current_material = material;
  current_state = SYSTEM_WAITING;
  state_start_time = millis();
  
  String material_name = getMaterialName(material);
  Serial.print("🚀 Iniciando clasificación: ");
  Serial.println(material_name);
  
  // Encender motor
  setMotorSpeed(200); // 78% velocidad
  motor_running = true;
  
  Serial.println("⚙️ Motor encendido - Esperando detección...");
}

void startProcessing() {
  current_state = SYSTEM_PROCESSING;
  
  String material_name = getMaterialName(current_material);
  Serial.print("⚙️ Procesando ");
  Serial.println(material_name);
  
  // Apagar motor
  setMotorSpeed(0);
  motor_running = false;
  
  // Activar servo correspondiente
  if (pca9685_ok) {
    int servo_channel = current_material - 1; // 0, 1, 2
    Serial.print("🎯 Activando servo ");
    Serial.println(servo_channel);
    
    // Empuje
    pwm.setPWM(servo_channel, 0, SERVO_MAX);
    delay(1500);
    
    // Reposo
    pwm.setPWM(servo_channel, 0, SERVO_MIN);
  }
  
  // Actualizar estadísticas
  total_processed++;
  switch (current_material) {
    case 1: vidrio_count++; break;
    case 2: plastico_count++; break;
    case 3: metal_count++; break;
  }
  
  Serial.println("✅ Procesamiento completado");
  
  // Volver a estado de espera
  delay(1000);
  current_state = SYSTEM_WAITING;
  state_start_time = millis();
  
  // Reencender motor
  setMotorSpeed(200);
  motor_running = true;
  
  Serial.println("🔄 Sistema listo para siguiente objeto");
}

void stopSystem() {
  current_state = SYSTEM_IDLE;
  current_material = 0;
  
  // Apagar motor
  setMotorSpeed(0);
  motor_running = false;
  
  // Servos a reposo
  if (pca9685_ok) {
    pwm.setPWM(SERVO_VIDRIO, 0, SERVO_MIN);
    pwm.setPWM(SERVO_PLASTICO, 0, SERVO_MIN);
    pwm.setPWM(SERVO_METAL, 0, SERVO_MIN);
  }
  
  Serial.println("🛑 Sistema detenido");
}

// ===============================================================================
// LECTURA DE SENSORES
// ===============================================================================

void readSensors() {
  // Guardar estados anteriores
  pir1_last = pir1_state;
  pir2_last = pir2_state;
  pir3_last = pir3_state;
  
  // Leer estados actuales
  pir1_state = digitalRead(PIR1_PIN);
  pir2_state = digitalRead(PIR2_PIN);
  pir3_state = digitalRead(PIR3_PIN);
  
  // Contar detecciones (para diagnóstico)
  if (pir1_state && !pir1_last) pir1_count++;
  if (pir2_state && !pir2_last) pir2_count++;
  if (pir3_state && !pir3_last) pir3_count++;
}

// ===============================================================================
// CONTROL DE COMPONENTES
// ===============================================================================

void setMotorSpeed(int speed) {
  speed = constrain(speed, 0, 255);
  ledcWrite(pwm_channel, speed);
  motor_speed = speed;
}

void updateDisplay() {
  if (!oled_ok) return;
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  
  switch (current_state) {
    case SYSTEM_IDLE:
      display.setTextSize(2);
      display.setCursor(35, 8);
      display.println("IDLE");
      break;
      
    case SYSTEM_WAITING:
      display.println("ESPERANDO:");
      display.setTextSize(2);
      display.setCursor(getCenterX(getMaterialName(current_material), 2), 16);
      display.println(getMaterialName(current_material));
      break;
      
    case SYSTEM_PROCESSING:
      display.println("PROCESANDO:");
      display.setTextSize(2);
      display.setCursor(getCenterX(getMaterialName(current_material), 2), 16);
      display.println(getMaterialName(current_material));
      break;
      
    case SYSTEM_ERROR:
      display.setTextSize(2);
      display.setCursor(20, 8);
      display.println("ERROR");
      break;
  }
  
  display.display();
}

void updateStatusLED() {
  static unsigned long last_blink = 0;
  static bool led_state = false;
  
  unsigned long interval;
  
  switch (current_state) {
    case SYSTEM_IDLE:
      interval = 2000; // Parpadeo lento
      break;
    case SYSTEM_WAITING:
      interval = 500; // Parpadeo medio
      break;
    case SYSTEM_PROCESSING:
      interval = 100; // Parpadeo rápido
      break;
    case SYSTEM_ERROR:
      interval = 200; // Parpadeo muy rápido
      break;
  }
  
  if (millis() - last_blink > interval) {
    led_state = !led_state;
    digitalWrite(LED_PIN, led_state);
    last_blink = millis();
  }
}

void blinkError() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

// ===============================================================================
// COMANDOS SERIALES
// ===============================================================================

void processSerialCommand() {
  String command = Serial.readString();
  command.trim();
  
  if (command == "1") {
    startMaterial(1);
  } else if (command == "2") {
    startMaterial(2);
  } else if (command == "3") {
    startMaterial(3);
  } else if (command == "stop") {
    stopSystem();
  } else if (command == "status") {
    printFullStatus();
  } else if (command == "stats") {
    printStatistics();
  } else if (command == "reset") {
    resetStatistics();
  } else if (command == "test") {
    runSystemTest();
  } else if (command == "help") {
    printHelp();
  } else {
    Serial.println("❌ Comando no reconocido");
    Serial.println("💡 Envía 'help' para ver comandos disponibles");
  }
}

// ===============================================================================
// INFORMES Y DIAGNÓSTICOS
// ===============================================================================

void printHelp() {
  Serial.println("🎮 COMANDOS DISPONIBLES:");
  Serial.println("========================");
  Serial.println("- '1' = Clasificar VIDRIO");
  Serial.println("- '2' = Clasificar PLASTICO");
  Serial.println("- '3' = Clasificar METAL");
  Serial.println("- 'stop' = Detener sistema");
  Serial.println("- 'status' = Estado completo del sistema");
  Serial.println("- 'stats' = Estadísticas de procesamiento");
  Serial.println("- 'reset' = Resetear estadísticas");
  Serial.println("- 'test' = Prueba completa del sistema");
  Serial.println("- 'help' = Mostrar esta ayuda");
  Serial.println();
}

void printStatusReport() {
  Serial.print("📊 Estado: ");
  Serial.print(getStateName(current_state));
  
  if (current_state == SYSTEM_WAITING || current_state == SYSTEM_PROCESSING) {
    Serial.print(" (");
    Serial.print(getMaterialName(current_material));
    Serial.print(")");
  }
  
  Serial.print(" | Motor: ");
  Serial.print(motor_running ? "ON" : "OFF");
  
  Serial.print(" | PIR: [");
  Serial.print(pir1_state ? "1" : "0");
  Serial.print("|");
  Serial.print(pir2_state ? "1" : "0");
  Serial.print("|");
  Serial.print(pir3_state ? "1" : "0");
  Serial.print("]");
  
  Serial.print(" | Procesados: ");
  Serial.println(total_processed);
}

void printFullStatus() {
  Serial.println("📋 ESTADO COMPLETO DEL SISTEMA");
  Serial.println("===============================");
  Serial.println();
  
  Serial.print("🚦 Estado: ");
  Serial.println(getStateName(current_state));
  
  if (current_material > 0) {
    Serial.print("🎯 Material activo: ");
    Serial.println(getMaterialName(current_material));
  }
  
  Serial.println();
  Serial.println("⚙️ HARDWARE:");
  Serial.print("- Motor: ");
  Serial.print(motor_running ? "FUNCIONANDO" : "PARADO");
  Serial.print(" (");
  Serial.print((motor_speed * 100) / 255);
  Serial.println("%)");
  
  Serial.print("- OLED: ");
  Serial.println(oled_ok ? "OK ✅" : "ERROR ❌");
  
  Serial.print("- PCA9685: ");
  Serial.println(pca9685_ok ? "OK ✅" : "ERROR ❌");
  
  Serial.println();
  Serial.println("🔍 SENSORES PIR:");
  Serial.print("- PIR1 (VIDRIO): ");
  Serial.print(pir1_state ? "DETECTANDO ⚡" : "REPOSO 💤");
  Serial.print(" (Detecciones: ");
  Serial.print(pir1_count);
  Serial.println(")");
  
  Serial.print("- PIR2 (PLASTICO): ");
  Serial.print(pir2_state ? "DETECTANDO ⚡" : "REPOSO 💤");
  Serial.print(" (Detecciones: ");
  Serial.print(pir2_count);
  Serial.println(")");
  
  Serial.print("- PIR3 (METAL): ");
  Serial.print(pir3_state ? "DETECTANDO ⚡" : "REPOSO 💤");
  Serial.print(" (Detecciones: ");
  Serial.print(pir3_count);
  Serial.println(")");
  
  Serial.println();
  printStatistics();
}

void printStatistics() {
  Serial.println("📈 ESTADÍSTICAS:");
  Serial.print("- Total procesado: ");
  Serial.println(total_processed);
  Serial.print("- VIDRIO: ");
  Serial.println(vidrio_count);
  Serial.print("- PLASTICO: ");
  Serial.println(plastico_count);
  Serial.print("- METAL: ");
  Serial.println(metal_count);
  Serial.print("- Errores: ");
  Serial.println(error_count);
  Serial.print("- Tiempo funcionamiento: ");
  Serial.print(millis() / 1000);
  Serial.println(" segundos");
  Serial.println();
}

void resetStatistics() {
  total_processed = 0;
  vidrio_count = 0;
  plastico_count = 0;
  metal_count = 0;
  error_count = 0;
  pir1_count = 0;
  pir2_count = 0;
  pir3_count = 0;
  
  Serial.println("🔄 Estadísticas reseteadas");
}

void runSystemTest() {
  Serial.println("🧪 INICIANDO PRUEBA COMPLETA DEL SISTEMA");
  Serial.println("========================================");
  
  // Asegurar que sistema esté parado
  stopSystem();
  delay(1000);
  
  // Prueba 1: Motor
  Serial.println("1️⃣ Probando motor...");
  setMotorSpeed(100);
  delay(2000);
  setMotorSpeed(200);
  delay(2000);
  setMotorSpeed(0);
  Serial.println("   ✅ Motor OK");
  
  // Prueba 2: Servos
  if (pca9685_ok) {
    Serial.println("2️⃣ Probando servos...");
    for (int i = 0; i < 3; i++) {
      Serial.print("   Servo ");
      Serial.println(i);
      pwm.setPWM(i, 0, SERVO_MAX);
      delay(1000);
      pwm.setPWM(i, 0, SERVO_MIN);
      delay(500);
    }
    Serial.println("   ✅ Servos OK");
  }
  
  // Prueba 3: Display
  if (oled_ok) {
    Serial.println("3️⃣ Probando display...");
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(30, 8);
    display.println("TEST");
    display.display();
    delay(2000);
    Serial.println("   ✅ Display OK");
  }
  
  // Prueba 4: Sensores PIR
  Serial.println("4️⃣ Probando sensores PIR...");
  Serial.println("   Mueva la mano frente a cada sensor...");
  
  unsigned long test_start = millis();
  bool pir1_tested = false, pir2_tested = false, pir3_tested = false;
  
  while (millis() - test_start < 10000) { // 10 segundos para probar
    readSensors();
    
    if (pir1_state && !pir1_tested) {
      Serial.println("   ✅ PIR1 (VIDRIO) detectado");
      pir1_tested = true;
    }
    if (pir2_state && !pir2_tested) {
      Serial.println("   ✅ PIR2 (PLASTICO) detectado");
      pir2_tested = true;
    }
    if (pir3_state && !pir3_tested) {
      Serial.println("   ✅ PIR3 (METAL) detectado");
      pir3_tested = true;
    }
    
    if (pir1_tested && pir2_tested && pir3_tested) break;
    delay(100);
  }
  
  if (!pir1_tested) Serial.println("   ⚠️ PIR1 no probado");
  if (!pir2_tested) Serial.println("   ⚠️ PIR2 no probado");
  if (!pir3_tested) Serial.println("   ⚠️ PIR3 no probado");
  
  Serial.println();
  Serial.println("🎉 PRUEBA COMPLETA FINALIZADA");
  
  // Volver a estado idle
  updateDisplay();
}

// ===============================================================================
// FUNCIONES AUXILIARES
// ===============================================================================

String getMaterialName(int material) {
  switch (material) {
    case 1: return "VIDRIO";
    case 2: return "PLASTICO";
    case 3: return "METAL";
    default: return "NONE";
  }
}

String getStateName(SystemState state) {
  switch (state) {
    case SYSTEM_IDLE: return "IDLE";
    case SYSTEM_WAITING: return "WAITING";
    case SYSTEM_PROCESSING: return "PROCESSING";
    case SYSTEM_ERROR: return "ERROR";
    default: return "UNKNOWN";
  }
}

int getCenterX(String text, int textSize = 1) {
  int char_width = 6 * textSize;
  int text_width = text.length() * char_width;
  return (SCREEN_WIDTH - text_width) / 2;
}

/*
===============================================================================
                                MANUAL DE USO
===============================================================================

🎯 OBJETIVO:
Probar todos los componentes del sistema UpCycle Pro funcionando de manera
integrada, simulando el proceso completo de clasificación.

🔧 CONFIGURACIÓN COMPLETA:
1. Conectar todos los componentes según documentación individual
2. Verificar alimentación (3.3V para OLED, 5V para PCA9685, 12V para motor)
3. Subir este sketch al ESP32
4. Abrir Monitor Serial (115200 baud)

📋 PROCEDIMIENTO DE PRUEBA:

INICIALIZACIÓN:
1. Sistema verifica todos los componentes al iniciar
2. Reporta estado de OLED y PCA9685
3. Muestra comandos disponibles

PRUEBA AUTOMÁTICA:
1. Enviar 'test' para ejecutar prueba completa automática
2. Sistema prueba motor, servos, display y sensores PIR
3. Reporta resultados de cada componente

PRUEBA MANUAL DE CLASIFICACIÓN:
1. Enviar '1' para modo VIDRIO
   - Motor se enciende
   - Display muestra "ESPERANDO: VIDRIO"
   - Mover mano frente a PIR1 (GPIO26)
   - Motor se apaga, servo empuja, vuelve a esperar

2. Enviar '2' para modo PLASTICO
   - Similar al anterior pero con PIR2 (GPIO27) y servo 1

3. Enviar '3' para modo METAL
   - Similar al anterior pero con PIR3 (GPIO14) y servo 2

COMANDOS DE CONTROL:
- 'stop': Detener sistema y volver a IDLE
- 'status': Ver estado completo del sistema
- 'stats': Ver estadísticas de procesamiento
- 'reset': Resetear contadores
- 'help': Ver lista de comandos

✅ FUNCIONAMIENTO CORRECTO:
- Todos los componentes se inicializan sin errores
- Motor responde a comandos de encendido/apagado
- Servos se mueven correctamente cuando se activan
- Display muestra las pantallas correspondientes
- PIR sensors detectan movimiento y activan procesamiento
- Sistema mantiene estadísticas correctas

❌ PROBLEMAS COMUNES:

"OLED no detectado":
- Verificar conexiones I2C (SDA=21, SCL=22)
- Verificar alimentación 3.3V (NO 5V)
- Probar dirección alternativa 0x3D

"PCA9685 no detectado":
- Verificar conexiones I2C compartidas
- Verificar alimentación 5V del PCA9685
- Verificar que no hay conflicto de direcciones

"PIR no detecta":
- Verificar conexiones GPIO (26, 27, 14)
- Verificar alimentación 5V de sensores
- Esperar tiempo de estabilización (2-5 minutos)

"Motor no funciona":
- Verificar conexión del transistor 2N222
- Verificar alimentación 12V del motor
- Verificar que motor no consume >500mA

"Servos no se mueven":
- Verificar que PCA9685 esté inicializado
- Verificar alimentación 5V de servos
- Verificar conexiones de servos a canales correctos

📊 INTERPRETACIÓN DE RESULTADOS:

🟢 Sistema completamente funcional:
- Todos los componentes detectados e inicializados
- Clasificación automática funciona correctamente
- Estadísticas se mantienen precisas

🟡 Sistema parcialmente funcional:
- Algunos componentes no detectados pero sistema funciona
- Funcionalidad limitada pero usable para pruebas

🔴 Sistema no funcional:
- Componentes críticos no responden
- Revisar conexiones y alimentación

🚀 SIGUIENTE PASO:
Una vez verificado el funcionamiento integrado, el sistema está listo
para implementar el código final con la librería SimpleClassifier.

===============================================================================
*/