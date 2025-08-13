/*
  TEST PIR MANAGER - UpCycle Pro
  ===============================
  
  Este sketch prueba la clase PIRManager que maneja los 3 sensores PIR
  del sistema de clasificación usando las librerías del proyecto.
  
  Librerías utilizadas:
  - PIRManager.h/.cpp (del proyecto UpCycle Pro)
  
  Conexiones:
  - PIR1 (VIDRIO):  GPIO26
  - PIR2 (PLASTICO): GPIO27  
  - PIR3 (METAL):   GPIO14
  - LED onboard:    GPIO2 (indicador visual)
  
  Funcionamiento:
  - Usa la clase PIRManager para gestión de sensores
  - Debounce automático de 500ms
  - Detección de activaciones con reportes
  - Control por comandos seriales
*/

// Incluir la librería del proyecto
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/PIRManager.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/PIRManager.cpp"

// LED indicador
#define LED_PIN 2

// Crear objeto PIRManager
PIRManager pirSensors;

// Variables de control
unsigned long last_report = 0;
unsigned long last_led_off = 0;
bool led_active = false;
bool auto_report = true;
unsigned long report_interval = 3000; // 3 segundos

// Contadores de detecciones
unsigned long pir1_detections = 0;
unsigned long pir2_detections = 0;
unsigned long pir3_detections = 0;

void setup() {
  Serial.begin(115200);
  
  // Configurar LED indicador
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println();
  Serial.println("🔍 TEST PIR MANAGER - UpCycle Pro");
  Serial.println("==================================");
  Serial.println();
  
  Serial.println("📋 Usando librerías del proyecto:");
  Serial.println("- PIRManager.h/.cpp");
  Serial.println();
  
  // Inicializar PIRManager
  Serial.println("🔄 Inicializando PIRManager...");
  pirSensors.begin();
  
  // Tiempo de estabilización
  Serial.println("⏳ Esperando estabilización de sensores (5 segundos)...");
  delay(5000);
  
  Serial.println("✅ Sistema listo - PIRManager funcionando");
  Serial.println();
  
  Serial.println("🎯 Configuración de sensores:");
  Serial.println("- PIR1 (VIDRIO):  GPIO26 - checkPIR1()");
  Serial.println("- PIR2 (PLASTICO): GPIO27 - checkPIR2()");
  Serial.println("- PIR3 (METAL):   GPIO14 - checkPIR3()");
  Serial.println("- Debounce automático: 500ms");
  Serial.println();
  
  printHelp();
}

void loop() {
  // Procesar comandos seriales
  if (Serial.available()) {
    processCommand();
  }
  
  // Verificar detecciones usando PIRManager
  checkPIRDetections();
  
  // Manejar LED indicador
  handleLED();
  
  // Reporte automático periódico
  if (auto_report && (millis() - last_report > report_interval)) {
    printStatusReport();
    last_report = millis();
  }
  
  delay(50);
}

void checkPIRDetections() {
  // Usar métodos de PIRManager para verificar activaciones
  if (pirSensors.checkPIR1()) {
    pir1_detections++;
    activateLED();
    Serial.print("🔵 PIR1 (VIDRIO) activado! Total detecciones: ");
    Serial.println(pir1_detections);
  }
  
  if (pirSensors.checkPIR2()) {
    pir2_detections++;
    activateLED();
    Serial.print("🟠 PIR2 (PLASTICO) activado! Total detecciones: ");
    Serial.println(pir2_detections);
  }
  
  if (pirSensors.checkPIR3()) {
    pir3_detections++;
    activateLED();
    Serial.print("⚫ PIR3 (METAL) activado! Total detecciones: ");
    Serial.println(pir3_detections);
  }
}

void processCommand() {
  String command = Serial.readString();
  command.trim();
  command.toLowerCase();
  
  if (command == "status" || command == "s") {
    printFullStatus();
  } else if (command == "report" || command == "r") {
    printStatusReport();
  } else if (command == "auto" || command == "a") {
    auto_report = !auto_report;
    Serial.print("🔄 Reporte automático: ");
    Serial.println(auto_report ? "ACTIVADO" : "DESACTIVADO");
  } else if (command == "reset") {
    resetCounters();
  } else if (command == "test" || command == "t") {
    runDetectionTest();
  } else if (command == "pir") {
    // Usar método de PIRManager para mostrar estado
    Serial.println("📊 Estado PIR (usando PIRManager.printStatus()):");
    pirSensors.printStatus();
  } else if (command == "help" || command == "h") {
    printHelp();
  } else {
    Serial.println("❌ Comando no reconocido");
    Serial.println("💡 Envía 'help' para ver comandos disponibles");
  }
}

void activateLED() {
  digitalWrite(LED_PIN, HIGH);
  led_active = true;
  last_led_off = millis() + 1000; // LED encendido por 1 segundo
}

void handleLED() {
  if (led_active && millis() > last_led_off) {
    digitalWrite(LED_PIN, LOW);
    led_active = false;
  }
}

void printHelp() {
  Serial.println("🎮 COMANDOS DISPONIBLES:");
  Serial.println("========================");
  Serial.println("- 'status' o 's' = Estado completo del sistema");
  Serial.println("- 'report' o 'r' = Reporte rápido de detecciones");
  Serial.println("- 'auto' o 'a' = Activar/desactivar reporte automático");
  Serial.println("- 'reset' = Resetear contadores de detecciones");
  Serial.println("- 'test' o 't' = Ejecutar prueba de detección guiada");
  Serial.println("- 'pir' = Estado PIR usando PIRManager.printStatus()");
  Serial.println("- 'help' o 'h' = Mostrar esta ayuda");
  Serial.println();
  Serial.println("💡 Mueve la mano frente a cada sensor para probar");
  Serial.println();
}

void printStatusReport() {
  Serial.print("📊 Detecciones: PIR1=");
  Serial.print(pir1_detections);
  Serial.print(" | PIR2=");
  Serial.print(pir2_detections);
  Serial.print(" | PIR3=");
  Serial.print(pir3_detections);
  Serial.print(" | Total=");
  Serial.print(pir1_detections + pir2_detections + pir3_detections);
  Serial.print(" | Tiempo: ");
  Serial.print(millis() / 1000);
  Serial.println("s");
}

void printFullStatus() {
  Serial.println("📋 ESTADO COMPLETO - PIR MANAGER");
  Serial.println("=================================");
  Serial.println();
  
  Serial.println("🔧 Configuración PIRManager:");
  Serial.println("- Clase: PIRManager del proyecto UpCycle Pro");
  Serial.println("- Debounce automático: 500ms entre detecciones");
  Serial.println("- Métodos: checkPIR1(), checkPIR2(), checkPIR3()");
  Serial.println();
  
  Serial.println("📊 Estadísticas de detecciones:");
  Serial.print("- PIR1 (VIDRIO - GPIO26): ");
  Serial.print(pir1_detections);
  Serial.println(" detecciones");
  Serial.print("- PIR2 (PLASTICO - GPIO27): ");
  Serial.print(pir2_detections);
  Serial.println(" detecciones");
  Serial.print("- PIR3 (METAL - GPIO14): ");
  Serial.print(pir3_detections);
  Serial.println(" detecciones");
  Serial.print("- Total: ");
  Serial.print(pir1_detections + pir2_detections + pir3_detections);
  Serial.println(" detecciones");
  Serial.println();
  
  Serial.print("⚙️ Configuración:");
  Serial.print(" Reporte automático=");
  Serial.print(auto_report ? "ON" : "OFF");
  Serial.print(" | Intervalo=");
  Serial.print(report_interval / 1000);
  Serial.println("s");
  Serial.println();
  
  // Usar método de la clase PIRManager
  Serial.println("🔍 Estado actual PIR (PIRManager.printStatus()):");
  pirSensors.printStatus();
  Serial.println();
}

void resetCounters() {
  pir1_detections = 0;
  pir2_detections = 0;
  pir3_detections = 0;
  
  Serial.println("🔄 Contadores de detecciones reseteados");
  Serial.println("📊 PIR1=0 | PIR2=0 | PIR3=0");
}

void runDetectionTest() {
  Serial.println("🧪 PRUEBA DE DETECCIÓN GUIADA");
  Serial.println("==============================");
  Serial.println();
  
  Serial.println("Esta prueba verificará cada sensor PIR individualmente");
  Serial.println("usando los métodos de la clase PIRManager.");
  Serial.println();
  
  // Test PIR1
  Serial.println("1️⃣ Probando PIR1 (VIDRIO - GPIO26)");
  Serial.println("   Mueva la mano frente al sensor PIR1...");
  Serial.println("   Esperando activación (timeout 10 segundos)");
  
  unsigned long test_start = millis();
  bool pir1_detected = false;
  
  while (millis() - test_start < 10000 && !pir1_detected) {
    if (pirSensors.checkPIR1()) {
      Serial.println("   ✅ PIR1 detectado correctamente!");
      pir1_detected = true;
      activateLED();
    }
    delay(100);
  }
  
  if (!pir1_detected) {
    Serial.println("   ⚠️ PIR1 no detectado en 10 segundos");
  }
  
  delay(1000);
  
  // Test PIR2
  Serial.println();
  Serial.println("2️⃣ Probando PIR2 (PLASTICO - GPIO27)");
  Serial.println("   Mueva la mano frente al sensor PIR2...");
  Serial.println("   Esperando activación (timeout 10 segundos)");
  
  test_start = millis();
  bool pir2_detected = false;
  
  while (millis() - test_start < 10000 && !pir2_detected) {
    if (pirSensors.checkPIR2()) {
      Serial.println("   ✅ PIR2 detectado correctamente!");
      pir2_detected = true;
      activateLED();
    }
    delay(100);
  }
  
  if (!pir2_detected) {
    Serial.println("   ⚠️ PIR2 no detectado en 10 segundos");
  }
  
  delay(1000);
  
  // Test PIR3
  Serial.println();
  Serial.println("3️⃣ Probando PIR3 (METAL - GPIO14)");
  Serial.println("   Mueva la mano frente al sensor PIR3...");
  Serial.println("   Esperando activación (timeout 10 segundos)");
  
  test_start = millis();
  bool pir3_detected = false;
  
  while (millis() - test_start < 10000 && !pir3_detected) {
    if (pirSensors.checkPIR3()) {
      Serial.println("   ✅ PIR3 detectado correctamente!");
      pir3_detected = true;
      activateLED();
    }
    delay(100);
  }
  
  if (!pir3_detected) {
    Serial.println("   ⚠️ PIR3 no detectado en 10 segundos");
  }
  
  // Resumen del test
  Serial.println();
  Serial.println("📋 RESUMEN DE LA PRUEBA:");
  Serial.print("- PIR1 (VIDRIO): ");
  Serial.println(pir1_detected ? "✅ OK" : "❌ FALLO");
  Serial.print("- PIR2 (PLASTICO): ");
  Serial.println(pir2_detected ? "✅ OK" : "❌ FALLO");
  Serial.print("- PIR3 (METAL): ");
  Serial.println(pir3_detected ? "✅ OK" : "❌ FALLO");
  
  int working_sensors = (pir1_detected ? 1 : 0) + (pir2_detected ? 1 : 0) + (pir3_detected ? 1 : 0);
  Serial.print("- Sensores funcionando: ");
  Serial.print(working_sensors);
  Serial.println("/3");
  
  if (working_sensors == 3) {
    Serial.println("🎉 ¡Todos los sensores PIR funcionan correctamente!");
  } else if (working_sensors > 0) {
    Serial.println("⚠️ Algunos sensores no respondieron. Verificar conexiones.");
  } else {
    Serial.println("❌ Ningún sensor respondió. Verificar alimentación y conexiones.");
  }
  
  Serial.println();
  Serial.println("🔄 Volviendo al monitoreo normal...");
  Serial.println();
}

/*
===============================================================================
                                MANUAL DE USO
===============================================================================

🎯 OBJETIVO:
Probar la clase PIRManager del proyecto UpCycle Pro que gestiona los 3 sensores
PIR con debounce automático y métodos específicos para cada sensor.

📚 LIBRERÍAS UTILIZADAS:
- PIRManager.h/.cpp (del proyecto UpCycle Pro)
- Métodos: begin(), checkPIR1(), checkPIR2(), checkPIR3(), printStatus()

🔧 CONFIGURACIÓN:
1. Conectar sensores PIR según documentación del proyecto:
   - PIR1 (VIDRIO): GPIO26
   - PIR2 (PLASTICO): GPIO27
   - PIR3 (METAL): GPIO14
   - Alimentación: 5V para todos los sensores

2. Subir este sketch al ESP32
3. Abrir Monitor Serial (115200 baud)

📋 CARACTERÍSTICAS DE PIRManager:
- Debounce automático de 500ms entre detecciones
- Métodos específicos por sensor (checkPIR1, checkPIR2, checkPIR3)
- Control de estados anteriores para detectar activaciones
- Método printStatus() para diagnóstico

🎮 COMANDOS DISPONIBLES:
- 'status' o 's': Estado completo del sistema
- 'report' o 'r': Reporte rápido de detecciones
- 'auto' o 'a': Activar/desactivar reporte automático
- 'reset': Resetear contadores
- 'test' o 't': Prueba guiada de cada sensor
- 'pir': Estado usando PIRManager.printStatus()
- 'help' o 'h': Mostrar ayuda

📋 PROCEDIMIENTO DE PRUEBA:

PRUEBA AUTOMÁTICA:
1. Sistema monitorea continuamente los 3 sensores
2. Reporte automático cada 3 segundos
3. LED parpadea cuando cualquier sensor detecta

PRUEBA MANUAL:
1. Enviar 'test' para prueba guiada
2. Sistema pide mover mano frente a cada sensor
3. 10 segundos timeout por sensor
4. Reporte final de sensores funcionando

VERIFICACIÓN INDIVIDUAL:
1. Mover mano frente a PIR1: debe mostrar "🔵 PIR1 (VIDRIO) activado!"
2. Mover mano frente a PIR2: debe mostrar "🟠 PIR2 (PLASTICO) activado!"
3. Mover mano frente a PIR3: debe mostrar "⚫ PIR3 (METAL) activado!"

✅ RESULTADOS ESPERADOS:
- PIRManager se inicializa correctamente
- Cada sensor detecta movimiento independientemente
- Debounce funciona (no múltiples detecciones rápidas)
- LED parpadea con cada detección
- Contadores incrementan correctamente
- printStatus() muestra información detallada

❌ PROBLEMAS Y SOLUCIONES:

"Sensor no detecta":
- Verificar conexión GPIO específico (26, 27, 14)
- Verificar alimentación 5V del sensor
- Esperar estabilización completa (2-5 minutos)
- Verificar que no esté bloqueado físicamente

"Múltiples detecciones rápidas":
- El debounce de 500ms debería prevenir esto
- Si persiste, problema en la librería PIRManager

"PIRManager no inicializa":
- Verificar que archivos .h y .cpp estén accesibles
- Verificar compilación sin errores
- Verificar configuración de pines en PIRManager.h

📊 INTERPRETACIÓN:
- Detecciones consistentes: Sensor funcionando correctamente
- Sin detecciones: Problema de hardware o conexión
- Detecciones erráticas: Interferencias o problema de alimentación

🎯 VENTAJAS DE USAR PIRManager:
- Debounce automático integrado
- Métodos específicos por aplicación
- Gestión de estados interna
- Diagnóstico integrado con printStatus()
- Código más limpio y mantenible

===============================================================================
*/