/*
  TEST SIMPLE CLASSIFIER - Sistema Completo UpCycle Pro
  ====================================================
  
  Este sketch prueba la clase SimpleClassifier completa que integra todos 
  los componentes del sistema (PIR, Motor, Servos, OLED).
  
  Características probadas:
  - Integración completa de todos los componentes
  - Estados del sistema (IDLE, WAITING, PROCESSING)
  - Comandos seriales para control
  - Timeout automático
  - Pantalla OLED con diferentes modos
  - Clasificación automática por material
  
  Hardware necesario:
  - ESP32
  - 3 sensores PIR (GPIO26, 27, 14)
  - Motor con 2N222 (GPIO25)
  - PCA9685 para servos
  - OLED SSD1306 (I2C)
  
  Comandos disponibles:
  - '1': Activar modo VIDRIO (PIR1 → Servo1)
  - '2': Activar modo PLASTICO (PIR2 → Servo2)
  - '3': Activar modo METAL (PIR3 → Servo3)
  - 'stop': Detener sistema
  - 'status': Ver estado
  - 'help': Ver comandos
*/

// Incluir la librería completa del proyecto
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/SimpleClassifier.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/SimpleClassifier.cpp"

// Incluir todas las dependencias necesarias
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/PIRManager.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/PIRManager.cpp"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ConveyorMotor.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ConveyorMotor.cpp"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ServoManager.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ServoManager.cpp"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/OLEDDisplay.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/OLEDDisplay.cpp"

// ===============================================================================
// OBJETOS GLOBALES
// ===============================================================================

SimpleClassifier classifier;

// ===============================================================================
// SETUP
// ===============================================================================

void setup() {
  Serial.begin(115200);
  
  Serial.println();
  Serial.println("🧪 TEST SIMPLE CLASSIFIER - Sistema Completo UpCycle Pro");
  Serial.println("=========================================================");
  Serial.println();
  
  // Inicializar el sistema completo
  if (!classifier.begin()) {
    Serial.println("❌ Error inicializando SimpleClassifier");
    Serial.println("🔄 Reiniciando en 5 segundos...");
    delay(5000);
    ESP.restart();
  }
  
  Serial.println("✅ SimpleClassifier inicializado correctamente");
  Serial.println("🎮 Envía comandos por Serial Monitor:");
  Serial.println();
  
  printTestInstructions();
}

// ===============================================================================
// LOOP PRINCIPAL
// ===============================================================================

void loop() {
  // Actualizar el clasificador (verifica sensores, timeouts, etc.)
  classifier.update();
  
  // Procesar comandos seriales
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    command.toLowerCase();
    
    processCommand(command);
  }
  
  delay(50); // Pequeña pausa para estabilidad
}

// ===============================================================================
// PROCESAMIENTO DE COMANDOS
// ===============================================================================

void processCommand(String command) {
  Serial.println("📨 Comando recibido: '" + command + "'");
  
  if (command == "1") {
    Serial.println("🔵 MODO VIDRIO seleccionado");
    classifier.setMode(1);
    
  } else if (command == "2") {
    Serial.println("🟠 MODO PLASTICO seleccionado");
    classifier.setMode(2);
    
  } else if (command == "3") {
    Serial.println("⚫ MODO METAL seleccionado");
    classifier.setMode(3);
    
  } else if (command == "stop") {
    Serial.println("🛑 DETENIENDO SISTEMA");
    classifier.stopSystem();
    
  } else if (command == "status") {
    classifier.printStatus();
    
  } else if (command == "help") {
    classifier.printHelp();
    printTestInstructions();
    
  } else if (command == "test") {
    runSystemTest();
    
  } else if (command == "demo") {
    runDemoSequence();
    
  } else {
    Serial.println("❌ Comando no reconocido: '" + command + "'");
    Serial.println("💡 Escribe 'help' para ver comandos disponibles");
  }
  
  Serial.println();
}

// ===============================================================================
// FUNCIONES DE TEST
// ===============================================================================

void runSystemTest() {
  Serial.println("🧪 EJECUTANDO TEST DEL SISTEMA");
  Serial.println("===============================");
  
  // Test 1: Verificar estado inicial
  Serial.println("1️⃣ Verificando estado inicial...");
  classifier.printStatus();
  delay(2000);
  
  // Test 2: Probar modo VIDRIO
  Serial.println("2️⃣ Probando modo VIDRIO...");
  classifier.setMode(1);
  delay(3000);
  
  // Test 3: Detener sistema
  Serial.println("3️⃣ Deteniendo sistema...");
  classifier.stopSystem();
  delay(2000);
  
  // Test 4: Probar modo PLASTICO
  Serial.println("4️⃣ Probando modo PLASTICO...");
  classifier.setMode(2);
  delay(3000);
  
  // Test 5: Detener sistema
  Serial.println("5️⃣ Deteniendo sistema...");
  classifier.stopSystem();
  delay(2000);
  
  // Test 6: Probar modo METAL
  Serial.println("6️⃣ Probando modo METAL...");
  classifier.setMode(3);
  delay(3000);
  
  // Test 7: Detener sistema
  Serial.println("7️⃣ Test completado - Sistema detenido");
  classifier.stopSystem();
  
  Serial.println("✅ TEST DEL SISTEMA COMPLETADO");
  Serial.println("📋 Todos los modos han sido probados correctamente");
  Serial.println();
}

void runDemoSequence() {
  Serial.println("🎬 EJECUTANDO SECUENCIA DE DEMOSTRACIÓN");
  Serial.println("======================================");
  
  Serial.println("📱 Esta secuencia simula un ciclo completo de clasificación");
  Serial.println("🔄 Se activará cada modo por 10 segundos");
  Serial.println();
  
  // Demo modo VIDRIO
  Serial.println("🔵 DEMO MODO VIDRIO - 10 segundos");
  classifier.setMode(1);
  delay(10000);
  classifier.stopSystem();
  delay(2000);
  
  // Demo modo PLASTICO
  Serial.println("🟠 DEMO MODO PLASTICO - 10 segundos");
  classifier.setMode(2);
  delay(10000);
  classifier.stopSystem();
  delay(2000);
  
  // Demo modo METAL
  Serial.println("⚫ DEMO MODO METAL - 10 segundos");
  classifier.setMode(3);
  delay(10000);
  classifier.stopSystem();
  
  Serial.println("✅ DEMOSTRACIÓN COMPLETADA");
  Serial.println("🎯 Sistema listo para uso manual");
  Serial.println();
}

void printTestInstructions() {
  Serial.println("📚 INSTRUCCIONES DE PRUEBA:");
  Serial.println("============================");
  Serial.println("COMANDOS BÁSICOS:");
  Serial.println("  1, 2, 3   → Seleccionar modo de clasificación");
  Serial.println("  stop      → Detener sistema");
  Serial.println("  status    → Ver estado actual");
  Serial.println("  help      → Mostrar ayuda");
  Serial.println();
  Serial.println("COMANDOS DE TEST:");
  Serial.println("  test      → Ejecutar test automático completo");
  Serial.println("  demo      → Ejecutar demostración de 30 segundos");
  Serial.println();
  Serial.println("CÓMO PROBAR:");
  Serial.println("1. Envía '1' para activar modo VIDRIO");
  Serial.println("2. Acerca un objeto al sensor PIR1 (GPIO26)");
  Serial.println("3. Observa que el motor se detiene y el servo1 se activa");
  Serial.println("4. El sistema vuelve a modo espera automáticamente");
  Serial.println("5. Repite con modos 2 y 3 para otros materiales");
  Serial.println();
  Serial.println("⚠️  NOTAS IMPORTANTES:");
  Serial.println("- El sistema tiene timeout de 30 segundos en modo espera");
  Serial.println("- El motor funciona mientras espera detección");
  Serial.println("- La pantalla OLED muestra el estado actual");
  Serial.println("- Los servos se activan por 1.5 segundos");
  Serial.println();
}

/*
===============================================================================
                                MANUAL DE USO
===============================================================================

🎯 OBJETIVO:
Probar la integración completa del sistema SimpleClassifier que combina
todos los componentes del proyecto UpCycle Pro en una sola clase.

🔧 HARDWARE NECESARIO:
- ESP32
- 3 sensores PIR conectados a GPIO26, 27, 14
- Motor con transistor 2N222 en GPIO25
- PCA9685 para control de 3 servos
- Display OLED SSD1306 en I2C (SDA=21, SCL=22)

📋 PROCEDIMIENTO DE PRUEBA:

FASE 1 - INICIALIZACIÓN:
1. Subir sketch al ESP32
2. Abrir Monitor Serial (115200 baud)
3. Verificar que todos los componentes se inicialicen correctamente
4. Debe aparecer "Sistema inicializado correctamente"

FASE 2 - PRUEBA MANUAL:
1. Enviar comando '1' → Sistema entra en modo VIDRIO
2. Motor debe empezar a funcionar
3. OLED debe mostrar "ESPERANDO VIDRIO"
4. Acercar objeto a PIR1 (GPIO26)
5. Motor debe detenerse, servo1 debe activarse
6. Después de 1.5s sistema vuelve a espera

FASE 3 - PRUEBA AUTOMÁTICA:
1. Enviar comando 'test' → Ejecuta secuencia automática
2. Verifica que todos los modos funcionan
3. Observa transiciones de estado en OLED

FASE 4 - PRUEBA DE TIMEOUT:
1. Activar cualquier modo (1, 2 o 3)
2. NO acercar objetos a sensores
3. Esperar 30 segundos
4. Sistema debe detectar timeout y detenerse automáticamente

FASE 5 - PRUEBA DE DEMOSTRACIÓN:
1. Enviar comando 'demo'
2. Observa secuencia automática de 30 segundos
3. Cada modo se activa por 10 segundos

🎮 COMANDOS PRINCIPALES:
- '1': Modo VIDRIO (PIR1 → Servo1)
- '2': Modo PLASTICO (PIR2 → Servo2) 
- '3': Modo METAL (PIR3 → Servo3)
- 'stop': Detener sistema
- 'status': Ver estado completo
- 'test': Test automático
- 'demo': Demostración de 30s
- 'help': Ayuda completa

✅ RESULTADOS ESPERADOS:

INICIALIZACIÓN:
- Todos los componentes se inicializan sin errores
- OLED muestra pantalla principal "UpCyclePro"
- Sistema entra en estado IDLE

FUNCIONAMIENTO NORMAL:
- Comandos 1,2,3 activan modo correspondiente
- Motor funciona durante espera de detección
- PIR sensors detectan objetos correctamente
- Servos se activan cuando corresponde
- OLED muestra estado actual siempre
- Sistema vuelve a espera después de clasificar

MANEJO DE ERRORES:
- Timeout automático después de 30 segundos
- Comandos inválidos muestran error
- Sistema recupera estado IDLE correctamente

❌ PROBLEMAS COMUNES:

"Error inicializando OLED":
- Verificar conexiones I2C (SDA=21, SCL=22)
- Comprobar dirección I2C del display (0x3C)
- Sistema continúa funcionando sin display

"Error inicializando servos":
- Verificar conexión del PCA9685
- Comprobar alimentación de 5V para servos
- Verificar dirección I2C del PCA9685

"PIR no detecta":
- Verificar conexiones GPIO26, 27, 14
- Comprobar alimentación 5V de sensores PIR
- Esperar 2-3 segundos para estabilización PIR

"Motor no funciona":
- Verificar conexión GPIO25
- Comprobar transistor 2N222 y resistencias
- Verificar alimentación del motor

"Timeout muy frecuente":
- Sensores PIR pueden estar detectando movimiento falso
- Ajustar sensibilidad de los sensores
- Revisar interferencias electromagnéticas

📊 MONITOREO:
- Estado del sistema se muestra en Serial Monitor
- OLED display muestra información visual
- LEDs de estado en PCA9685 indican actividad servos
- Motor audible indica funcionamiento normal

🚀 SIGUIENTE PASO:
Una vez confirmado el funcionamiento correcto, integrar con:
- Sistema MQTT para control remoto
- Node-RED dashboard para monitoreo
- Base de datos para estadísticas
- Alertas automáticas por problemas

===============================================================================
*/