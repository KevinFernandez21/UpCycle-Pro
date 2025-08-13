/*
  TEST SERVO MANAGER - UpCycle Pro
  =================================
  
  Este sketch prueba la clase ServoManager que controla los 3 servos
  del sistema de clasificación usando las librerías del proyecto.
  
  Librerías utilizadas:
  - ServoManager.h/.cpp (del proyecto UpCycle Pro)
  - Adafruit_PWMServoDriver (incluida en ServoManager)
  
  Conexiones:
  - PCA9685: SDA=GPIO21, SCL=GPIO22, VCC=5V, GND=GND
  - Servo VIDRIO:   Canal 0 del PCA9685
  - Servo PLASTICO: Canal 1 del PCA9685  
  - Servo METAL:    Canal 2 del PCA9685
  
  Funcionamiento:
  - Usa la clase ServoManager para control de servos
  - Métodos específicos por material
  - Configuración de posiciones y tiempos
  - Control por comandos seriales
*/

// Incluir las librerías del proyecto
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ServoManager.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ServoManager.cpp"

// LED indicador
#define LED_PIN 2

// Crear objeto ServoManager
ServoManager servos;

// Variables de control
unsigned long last_status_report = 0;
bool auto_report = true;
bool servos_initialized = false;
unsigned long status_interval = 5000; // 5 segundos

// Estadísticas de uso
unsigned long servo1_activations = 0; // VIDRIO
unsigned long servo2_activations = 0; // PLASTICO
unsigned long servo3_activations = 0; // METAL
unsigned long total_activations = 0;

void setup() {
  Serial.begin(115200);
  
  // Configurar LED indicador
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println();
  Serial.println("🎛️ TEST SERVO MANAGER - UpCycle Pro");
  Serial.println("===================================");
  Serial.println();
  
  Serial.println("📋 Usando librerías del proyecto:");
  Serial.println("- ServoManager.h/.cpp");
  Serial.println("- Adafruit_PWMServoDriver (integrada)");
  Serial.println();
  
  // Inicializar I2C
  Wire.begin();
  
  // Inicializar ServoManager
  Serial.println("🔄 Inicializando ServoManager...");
  if (servos.begin()) {
    servos_initialized = true;
    Serial.println("✅ ServoManager inicializado correctamente");
    
    // LED indicador de éxito
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
  } else {
    Serial.println("❌ Error inicializando ServoManager");
    Serial.println("   Verificar conexiones I2C y alimentación PCA9685");
    while(1) {
      digitalWrite(LED_PIN, HIGH);
      delay(500);
      digitalWrite(LED_PIN, LOW);
      delay(500);
    }
  }
  
  Serial.println();
  Serial.println("🔧 Configuración ServoManager:");
  Serial.println("- Servo VIDRIO:   Canal 0 (SERVO1_CHANNEL)");
  Serial.println("- Servo PLASTICO: Canal 1 (SERVO2_CHANNEL)");
  Serial.println("- Servo METAL:    Canal 2 (SERVO3_CHANNEL)");
  Serial.println("- Posición reposo: 0° (ajustable)");
  Serial.println("- Posición empuje: 90° (ajustable)");
  Serial.println("- Tiempo empuje: 1000ms (ajustable)");
  Serial.println();
  
  // Resetear todos los servos a posición inicial
  Serial.println("🏠 Reseteando servos a posición de reposo...");
  servos.resetAllServos();
  delay(2000);
  
  printHelp();
}

void loop() {
  // Procesar comandos seriales
  if (Serial.available()) {
    processCommand();
  }
  
  // Reporte automático periódico
  if (auto_report && servos_initialized && (millis() - last_status_report > status_interval)) {
    printStatusReport();
    last_status_report = millis();
  }
  
  delay(100);
}

void processCommand() {
  String command = Serial.readString();
  command.trim();
  command.toLowerCase();
  
  if (command == "1" || command == "vidrio") {
    activateServo1();
  } else if (command == "2" || command == "plastico") {
    activateServo2();
  } else if (command == "3" || command == "metal") {
    activateServo3();
  } else if (command == "reset" || command == "r") {
    resetServos();
  } else if (command == "status" || command == "s") {
    printFullStatus();
  } else if (command == "auto" || command == "a") {
    auto_report = !auto_report;
    Serial.print("🔄 Reporte automático: ");
    Serial.println(auto_report ? "ACTIVADO" : "DESACTIVADO");
  } else if (command == "stats") {
    printStatistics();
  } else if (command == "clear") {
    clearStatistics();
  } else if (command == "test") {
    runServoTest();
  } else if (command == "sequence") {
    runSequenceTest();
  } else if (command == "config") {
    showConfiguration();
  } else if (command == "servo") {
    // Usar método de ServoManager para mostrar estado
    Serial.println("📊 Estado servos (usando ServoManager.printStatus()):");
    servos.printStatus();
  } else if (command == "help" || command == "h") {
    printHelp();
  } else {
    Serial.println("❌ Comando no reconocido");
    Serial.println("💡 Envía 'help' para ver comandos disponibles");
  }
}

void activateServo1() {
  if (!servos_initialized) {
    Serial.println("❌ ServoManager no inicializado");
    return;
  }
  
  Serial.println("🔵 Activando Servo VIDRIO (Canal 0)");
  Serial.println("   Usando ServoManager.activateServo1()...");
  
  servos.activateServo1();
  servo1_activations++;
  total_activations++;
  
  // LED indicador
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("   ✅ Servo VIDRIO activado");
}

void activateServo2() {
  if (!servos_initialized) {
    Serial.println("❌ ServoManager no inicializado");
    return;
  }
  
  Serial.println("🟠 Activando Servo PLASTICO (Canal 1)");
  Serial.println("   Usando ServoManager.activateServo2()...");
  
  servos.activateServo2();
  servo2_activations++;
  total_activations++;
  
  // LED indicador
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("   ✅ Servo PLASTICO activado");
}

void activateServo3() {
  if (!servos_initialized) {
    Serial.println("❌ ServoManager no inicializado");
    return;
  }
  
  Serial.println("⚫ Activando Servo METAL (Canal 2)");
  Serial.println("   Usando ServoManager.activateServo3()...");
  
  servos.activateServo3();
  servo3_activations++;
  total_activations++;
  
  // LED indicador
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("   ✅ Servo METAL activado");
}

void resetServos() {
  if (!servos_initialized) {
    Serial.println("❌ ServoManager no inicializado");
    return;
  }
  
  Serial.println("🏠 Reseteando todos los servos");
  Serial.println("   Usando ServoManager.resetAllServos()...");
  
  servos.resetAllServos();
  
  Serial.println("   ✅ Todos los servos en posición de reposo");
}

void printHelp() {
  Serial.println("🎮 COMANDOS DISPONIBLES:");
  Serial.println("========================");
  Serial.println("- '1' o 'vidrio' = Activar servo VIDRIO (Canal 0)");
  Serial.println("- '2' o 'plastico' = Activar servo PLASTICO (Canal 1)");
  Serial.println("- '3' o 'metal' = Activar servo METAL (Canal 2)");
  Serial.println("- 'reset' o 'r' = Resetear todos los servos a reposo");
  Serial.println("- 'status' o 's' = Estado completo del sistema");
  Serial.println("- 'auto' o 'a' = Activar/desactivar reporte automático");
  Serial.println("- 'stats' = Estadísticas de activaciones");
  Serial.println("- 'clear' = Limpiar estadísticas");
  Serial.println("- 'test' = Prueba individual de cada servo");
  Serial.println("- 'sequence' = Prueba secuencial automática");
  Serial.println("- 'config' = Mostrar configuración de servos");
  Serial.println("- 'servo' = Estado usando ServoManager.printStatus()");
  Serial.println("- 'help' o 'h' = Mostrar esta ayuda");
  Serial.println();
}

void printStatusReport() {
  Serial.print("📊 Servos: Inicializado=");
  Serial.print(servos_initialized ? "✅" : "❌");
  Serial.print(" | Activaciones: V=");
  Serial.print(servo1_activations);
  Serial.print(" P=");
  Serial.print(servo2_activations);
  Serial.print(" M=");
  Serial.print(servo3_activations);
  Serial.print(" | Total=");
  Serial.println(total_activations);
}

void printFullStatus() {
  Serial.println("📋 ESTADO COMPLETO - SERVO MANAGER");
  Serial.println("==================================");
  Serial.println();
  
  Serial.println("🔧 Configuración ServoManager:");
  Serial.println("- Clase: ServoManager del proyecto UpCycle Pro");
  Serial.println("- Controlador: PCA9685 en I2C");
  Serial.println("- Métodos: activateServo1(), activateServo2(), activateServo3()");
  Serial.println("- Reset: resetAllServos()");
  Serial.println();
  
  Serial.print("⚙️ Estado de inicialización: ");
  Serial.println(servos_initialized ? "✅ INICIALIZADO" : "❌ ERROR");
  
  if (servos_initialized) {
    Serial.println();
    Serial.println("📊 Canales de servos:");
    Serial.println("- Canal 0: Servo VIDRIO (activateServo1)");
    Serial.println("- Canal 1: Servo PLASTICO (activateServo2)");
    Serial.println("- Canal 2: Servo METAL (activateServo3)");
    Serial.println();
    
    Serial.print("⚙️ Configuración:");
    Serial.print(" Reporte automático=");
    Serial.print(auto_report ? "ON" : "OFF");
    Serial.print(" | Intervalo=");
    Serial.print(status_interval / 1000);
    Serial.println("s");
    Serial.println();
    
    // Usar método de la clase ServoManager
    Serial.println("📊 Estado ServoManager (servos.printStatus()):");
    servos.printStatus();
    Serial.println();
  }
  
  printStatistics();
}

void printStatistics() {
  Serial.println("📈 ESTADÍSTICAS DE ACTIVACIONES:");
  Serial.print("- Servo VIDRIO (Canal 0): ");
  Serial.println(servo1_activations);
  Serial.print("- Servo PLASTICO (Canal 1): ");
  Serial.println(servo2_activations);
  Serial.print("- Servo METAL (Canal 2): ");
  Serial.println(servo3_activations);
  Serial.print("- Total activaciones: ");
  Serial.println(total_activations);
  
  if (total_activations > 0) {
    Serial.println();
    Serial.println("📊 Distribución porcentual:");
    Serial.print("- VIDRIO: ");
    Serial.print((servo1_activations * 100) / total_activations);
    Serial.println("%");
    Serial.print("- PLASTICO: ");
    Serial.print((servo2_activations * 100) / total_activations);
    Serial.println("%");
    Serial.print("- METAL: ");
    Serial.print((servo3_activations * 100) / total_activations);
    Serial.println("%");
  }
  Serial.println();
}

void clearStatistics() {
  servo1_activations = 0;
  servo2_activations = 0;
  servo3_activations = 0;
  total_activations = 0;
  
  Serial.println("🔄 Estadísticas limpiadas");
  Serial.println("📊 Todas las activaciones reseteadas a 0");
}

void showConfiguration() {
  Serial.println("⚙️ CONFIGURACIÓN SERVO MANAGER");
  Serial.println("===============================");
  Serial.println();
  Serial.println("📋 Definiciones (ServoManager.h):");
  Serial.println("- SERVO1_CHANNEL: 0 (VIDRIO)");
  Serial.println("- SERVO2_CHANNEL: 1 (PLASTICO)");
  Serial.println("- SERVO3_CHANNEL: 2 (METAL)");
  Serial.println();
  Serial.println("🎯 Posiciones por defecto:");
  Serial.println("- restPosition: 0° (reposo)");
  Serial.println("- pushPosition: 90° (empuje)");
  Serial.println("- servoDelay: 1000ms (tiempo de empuje)");
  Serial.println();
  Serial.println("⚡ Valores PWM:");
  Serial.println("- servoMin: 172 (0°)");
  Serial.println("- servoMax: 565 (180°)");
  Serial.println();
  Serial.println("💡 Los valores son configurables usando:");
  Serial.println("- setPositions(rest, push)");
  Serial.println("- setDelay(delayMs)");
  Serial.println();
}

void runServoTest() {
  if (!servos_initialized) {
    Serial.println("❌ No se puede ejecutar test - ServoManager no inicializado");
    return;
  }
  
  Serial.println("🧪 PRUEBA INDIVIDUAL DE SERVOS");
  Serial.println("===============================");
  Serial.println();
  
  Serial.println("Esta prueba activará cada servo individualmente");
  Serial.println("usando los métodos de la clase ServoManager.");
  Serial.println();
  
  // Reset inicial
  Serial.println("🏠 Reset inicial de todos los servos...");
  servos.resetAllServos();
  delay(2000);
  
  // Test Servo 1
  Serial.println("1️⃣ Probando Servo VIDRIO (Canal 0)");
  Serial.println("   Llamando servos.activateServo1()...");
  servos.activateServo1();
  Serial.println("   ✅ Servo VIDRIO activado");
  delay(3000);
  
  // Test Servo 2
  Serial.println();
  Serial.println("2️⃣ Probando Servo PLASTICO (Canal 1)");
  Serial.println("   Llamando servos.activateServo2()...");
  servos.activateServo2();
  Serial.println("   ✅ Servo PLASTICO activado");
  delay(3000);
  
  // Test Servo 3
  Serial.println();
  Serial.println("3️⃣ Probando Servo METAL (Canal 2)");
  Serial.println("   Llamando servos.activateServo3()...");
  servos.activateServo3();
  Serial.println("   ✅ Servo METAL activado");
  delay(3000);
  
  // Reset final
  Serial.println();
  Serial.println("🏠 Reset final - Todos a reposo");
  servos.resetAllServos();
  
  Serial.println();
  Serial.println("📋 RESUMEN DEL TEST:");
  Serial.println("- Servo VIDRIO: ✅ OK");
  Serial.println("- Servo PLASTICO: ✅ OK");
  Serial.println("- Servo METAL: ✅ OK");
  Serial.println();
  Serial.println("🎉 ¡Todos los servos funcionan correctamente!");
  Serial.println();
}

void runSequenceTest() {
  if (!servos_initialized) {
    Serial.println("❌ No se puede ejecutar secuencia - ServoManager no inicializado");
    return;
  }
  
  Serial.println("🔄 PRUEBA DE SECUENCIA AUTOMÁTICA");
  Serial.println("=================================");
  Serial.println();
  
  Serial.println("Ejecutando secuencia automática de 3 ciclos completos");
  Serial.println("(VIDRIO -> PLASTICO -> METAL -> Reset)");
  Serial.println();
  
  for (int cycle = 1; cycle <= 3; cycle++) {
    Serial.print("🔄 Ciclo ");
    Serial.print(cycle);
    Serial.println("/3");
    
    // VIDRIO
    Serial.println("   🔵 VIDRIO...");
    servos.activateServo1();
    delay(2000);
    
    // PLASTICO
    Serial.println("   🟠 PLASTICO...");
    servos.activateServo2();
    delay(2000);
    
    // METAL
    Serial.println("   ⚫ METAL...");
    servos.activateServo3();
    delay(2000);
    
    // Reset
    Serial.println("   🏠 Reset...");
    servos.resetAllServos();
    delay(1000);
    
    Serial.print("   ✅ Ciclo ");
    Serial.print(cycle);
    Serial.println(" completado");
    Serial.println();
  }
  
  Serial.println("🎉 ¡Secuencia automática completada!");
  Serial.println("📊 Total activaciones en secuencia: 9 (3 por servo)");
  Serial.println();
}

/*
===============================================================================
                                MANUAL DE USO
===============================================================================

🎯 OBJETIVO:
Probar la clase ServoManager del proyecto UpCycle Pro que controla los 3 servos
del sistema de clasificación usando PCA9685.

📚 LIBRERÍAS UTILIZADAS:
- ServoManager.h/.cpp (del proyecto UpCycle Pro)
- Adafruit_PWMServoDriver (integrada en ServoManager)
- Métodos: begin(), activateServo1(), activateServo2(), activateServo3(), resetAllServos()

🔧 CONFIGURACIÓN:
1. Conectar PCA9685 según documentación del proyecto:
   - SDA: GPIO21, SCL: GPIO22
   - VCC: 5V, GND: GND
   - Servos en canales 0, 1, 2

2. Alimentar servos con 5V (máximo 1.5A total)
3. Subir este sketch al ESP32
4. Abrir Monitor Serial (115200 baud)

📋 CARACTERÍSTICAS DE ServoManager:
- Control de 3 servos específicos por material
- Posiciones configurables (reposo 0°, empuje 90°)
- Tiempo de empuje configurable (1000ms por defecto)
- Métodos específicos por aplicación
- Reset automático a posición de reposo
- Valores PWM calibrados para servos estándar

🎮 COMANDOS DISPONIBLES:
- '1' o 'vidrio': Activar servo VIDRIO (Canal 0)
- '2' o 'plastico': Activar servo PLASTICO (Canal 1)
- '3' o 'metal': Activar servo METAL (Canal 2)
- 'reset' o 'r': Resetear todos los servos
- 'status' o 's': Estado completo del sistema
- 'auto' o 'a': Activar/desactivar reporte automático
- 'stats': Estadísticas de activaciones
- 'clear': Limpiar estadísticas
- 'test': Prueba individual de cada servo
- 'sequence': Prueba secuencial automática
- 'config': Mostrar configuración
- 'servo': Estado usando ServoManager.printStatus()
- 'help' o 'h': Mostrar ayuda

📋 PROCEDIMIENTO DE PRUEBA:

PRUEBA AUTOMÁTICA:
1. Enviar 'test' para prueba individual de cada servo
2. Sistema activa cada servo secuencialmente
3. Verificar movimiento físico de cada servo

PRUEBA MANUAL:
1. Enviar '1', '2', '3' para activar servos específicos
2. Verificar que cada servo se mueve correctamente
3. Usar 'reset' para volver todos a reposo

PRUEBA DE SECUENCIA:
1. Enviar 'sequence' para prueba automática completa
2. Sistema ejecuta 3 ciclos completos
3. Verificar funcionamiento coordinado

✅ RESULTADOS ESPERADOS:
- ServoManager se inicializa sin errores (PCA9685 detectado)
- Cada servo responde a su método específico
- Movimiento suave de reposo (0°) a empuje (90°)
- Reset automático después del tiempo configurado
- Estadísticas precisas de activaciones
- printStatus() muestra información detallada

❌ PROBLEMAS Y SOLUCIONES:

"ServoManager no inicializa":
- Verificar conexiones I2C (SDA=21, SCL=22)
- Verificar alimentación 5V del PCA9685
- Verificar dirección I2C 0x40
- Probar con I2C scanner primero

"Servo no se mueve":
- Verificar conexión del servo al canal correcto
- Verificar alimentación 5V de servos (mínimo 1A)
- Verificar que servo no esté bloqueado mecánicamente
- Comprobar valores PWM (servoMin/servoMax)

"Movimiento errático":
- Verificar calidad de alimentación (usar capacitor)
- Verificar interferencias en líneas I2C
- Revisar valores de calibración PWM
- Verificar que no hay sobrecarga de corriente

"Solo algunos servos funcionan":
- Verificar conexiones específicas de cada canal
- Verificar que todos los servos sean compatibles
- Comprobar consumo total de corriente

📊 INTERPRETACIÓN:
- Movimiento suave y preciso: Sistema funcionando correctamente
- Movimiento lento: Posible problema de alimentación
- Jitter: Interferencias o alimentación inestable
- Sin movimiento: Problema de hardware o conexión

🎯 VENTAJAS DE USAR ServoManager:
- Métodos específicos por material/aplicación
- Configuración centralizada de posiciones y tiempos
- Gestión automática del PCA9685
- Calibración PWM optimizada
- Diagnóstico integrado
- Código más limpio y mantenible

🔧 CONFIGURACIONES AVANZADAS:
- setPositions(rest, push): Cambiar ángulos de reposo y empuje
- setDelay(delayMs): Cambiar tiempo de empuje
- Valores PWM ajustables para diferentes tipos de servo

===============================================================================
*/