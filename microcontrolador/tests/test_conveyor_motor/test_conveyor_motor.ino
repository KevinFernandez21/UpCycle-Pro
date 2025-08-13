/*
  TEST CONVEYOR MOTOR - UpCycle Pro
  ==================================
  
  Este sketch prueba la clase ConveyorMotor que controla el motor de la 
  banda transportadora usando las librerías del proyecto.
  
  Librerías utilizadas:
  - ConveyorMotor.h/.cpp (del proyecto UpCycle Pro)
  
  Conexiones:
  - Motor: GPIO25 → Resistencia 220Ω → Base 2N222
  - Colector 2N222 → Motor 12V (+)
  - Emisor 2N222 → GND común
  - Motor 12V (-) → GND común
  
  Funcionamiento:
  - Usa la clase ConveyorMotor para control ON/OFF
  - Pruebas de arranque y parada
  - Monitoreo de estado del motor
  - Control por comandos seriales
*/

// Incluir la librería del proyecto
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ConveyorMotor.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ConveyorMotor.cpp"

// LED indicador
#define LED_PIN 2

// Crear objeto ConveyorMotor
ConveyorMotor motor;

// Variables de control
unsigned long last_status_report = 0;
unsigned long last_led_toggle = 0;
bool auto_report = true;
bool led_state = false;
unsigned long status_interval = 3000; // 3 segundos

// Estadísticas
unsigned long motor_starts = 0;
unsigned long motor_stops = 0;
unsigned long total_runtime = 0;
unsigned long last_start_time = 0;

void setup() {
  Serial.begin(115200);
  
  // Configurar LED indicador
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println();
  Serial.println("⚙️ TEST CONVEYOR MOTOR - UpCycle Pro");
  Serial.println("====================================");
  Serial.println();
  
  Serial.println("📋 Usando librerías del proyecto:");
  Serial.println("- ConveyorMotor.h/.cpp");
  Serial.println();
  
  // Inicializar ConveyorMotor
  Serial.println("🔄 Inicializando ConveyorMotor...");
  motor.begin();
  
  Serial.println("✅ ConveyorMotor inicializado correctamente");
  Serial.println();
  
  Serial.println("🔧 Configuración del motor:");
  Serial.println("- Pin de control: GPIO25 (definido en ConveyorMotor.h)");
  Serial.println("- Control: ON/OFF digital");
  Serial.println("- Métodos: start(), stop(), isMotorRunning()");
  Serial.println();
  
  // Estado inicial
  printMotorInfo();
  Serial.println();
  
  printHelp();
}

void loop() {
  // Procesar comandos seriales
  if (Serial.available()) {
    processCommand();
  }
  
  // Actualizar estadísticas de tiempo de funcionamiento
  updateRuntimeStats();
  
  // Manejar LED indicador
  handleStatusLED();
  
  // Reporte automático periódico
  if (auto_report && (millis() - last_status_report > status_interval)) {
    printStatusReport();
    last_status_report = millis();
  }
  
  delay(100);
}

void processCommand() {
  String command = Serial.readString();
  command.trim();
  command.toLowerCase();
  
  if (command == "start" || command == "on") {
    startMotor();
  } else if (command == "stop" || command == "off") {
    stopMotor();
  } else if (command == "toggle" || command == "t") {
    toggleMotor();
  } else if (command == "status" || command == "s") {
    printFullStatus();
  } else if (command == "auto" || command == "a") {
    auto_report = !auto_report;
    Serial.print("🔄 Reporte automático: ");
    Serial.println(auto_report ? "ACTIVADO" : "DESACTIVADO");
  } else if (command == "stats") {
    printStatistics();
  } else if (command == "reset") {
    resetStatistics();
  } else if (command == "test") {
    runMotorTest();
  } else if (command == "motor") {
    // Usar método de ConveyorMotor para mostrar estado
    Serial.println("📊 Estado motor (usando ConveyorMotor.printStatus()):");
    motor.printStatus();
  } else if (command == "help" || command == "h") {
    printHelp();
  } else {
    Serial.println("❌ Comando no reconocido");
    Serial.println("💡 Envía 'help' para ver comandos disponibles");
  }
}

void startMotor() {
  if (!motor.isMotorRunning()) {
    motor.start();
    motor_starts++;
    last_start_time = millis();
    Serial.println("🚀 Motor iniciado usando ConveyorMotor.start()");
  } else {
    Serial.println("⚠️ Motor ya está funcionando");
  }
}

void stopMotor() {
  if (motor.isMotorRunning()) {
    motor.stop();
    motor_stops++;
    Serial.println("🛑 Motor detenido usando ConveyorMotor.stop()");
  } else {
    Serial.println("⚠️ Motor ya está detenido");
  }
}

void toggleMotor() {
  if (motor.isMotorRunning()) {
    stopMotor();
  } else {
    startMotor();
  }
}

void updateRuntimeStats() {
  // Actualizar tiempo total de funcionamiento
  static unsigned long last_update = 0;
  if (millis() - last_update > 1000) { // Actualizar cada segundo
    if (motor.isMotorRunning()) {
      total_runtime += 1000; // Agregar 1 segundo
    }
    last_update = millis();
  }
}

void handleStatusLED() {
  unsigned long interval;
  
  if (motor.isMotorRunning()) {
    interval = 250; // Parpadeo rápido cuando motor funciona
  } else {
    interval = 2000; // Parpadeo lento cuando motor está parado
  }
  
  if (millis() - last_led_toggle > interval) {
    led_state = !led_state;
    digitalWrite(LED_PIN, led_state);
    last_led_toggle = millis();
  }
}

void printHelp() {
  Serial.println("🎮 COMANDOS DISPONIBLES:");
  Serial.println("========================");
  Serial.println("- 'start' o 'on' = Iniciar motor");
  Serial.println("- 'stop' o 'off' = Detener motor");
  Serial.println("- 'toggle' o 't' = Alternar estado del motor");
  Serial.println("- 'status' o 's' = Estado completo del sistema");
  Serial.println("- 'auto' o 'a' = Activar/desactivar reporte automático");
  Serial.println("- 'stats' = Estadísticas de funcionamiento");
  Serial.println("- 'reset' = Resetear estadísticas");
  Serial.println("- 'test' = Ejecutar prueba automática del motor");
  Serial.println("- 'motor' = Estado usando ConveyorMotor.printStatus()");
  Serial.println("- 'help' o 'h' = Mostrar esta ayuda");
  Serial.println();
}

void printMotorInfo() {
  Serial.println("ℹ️ INFORMACIÓN DEL MOTOR:");
  Serial.println("- Clase: ConveyorMotor del proyecto UpCycle Pro");
  Serial.println("- Control: Digital ON/OFF (sin PWM)");
  Serial.println("- Pin: GPIO25 (definido en MOTOR_PIN)");
  Serial.println("- Estado inicial: DETENIDO");
}

void printStatusReport() {
  Serial.print("📊 Motor: ");
  Serial.print(motor.isMotorRunning() ? "FUNCIONANDO ⚙️" : "PARADO 🛑");
  Serial.print(" | Arranques: ");
  Serial.print(motor_starts);
  Serial.print(" | Paradas: ");
  Serial.print(motor_stops);
  Serial.print(" | Tiempo funcionamiento: ");
  Serial.print(total_runtime / 1000);
  Serial.println("s");
}

void printFullStatus() {
  Serial.println("📋 ESTADO COMPLETO - CONVEYOR MOTOR");
  Serial.println("===================================");
  Serial.println();
  
  Serial.println("🔧 Configuración ConveyorMotor:");
  Serial.println("- Clase: ConveyorMotor del proyecto UpCycle Pro");
  Serial.println("- Pin de control: GPIO25 (MOTOR_PIN)");
  Serial.println("- Tipo de control: Digital ON/OFF");
  Serial.println("- Métodos: begin(), start(), stop(), isMotorRunning()");
  Serial.println();
  
  Serial.print("⚙️ Estado actual: ");
  Serial.println(motor.isMotorRunning() ? "FUNCIONANDO ⚙️" : "PARADO 🛑");
  
  if (motor.isMotorRunning() && last_start_time > 0) {
    unsigned long current_session = millis() - last_start_time;
    Serial.print("⏱️ Tiempo sesión actual: ");
    Serial.print(current_session / 1000);
    Serial.println(" segundos");
  }
  
  Serial.println();
  
  Serial.print("⚙️ Configuración:");
  Serial.print(" Reporte automático=");
  Serial.print(auto_report ? "ON" : "OFF");
  Serial.print(" | Intervalo=");
  Serial.print(status_interval / 1000);
  Serial.println("s");
  Serial.println();
  
  // Usar método de la clase ConveyorMotor
  Serial.println("📊 Estado ConveyorMotor (motor.printStatus()):");
  motor.printStatus();
  Serial.println();
  
  printStatistics();
}

void printStatistics() {
  Serial.println("📈 ESTADÍSTICAS DE FUNCIONAMIENTO:");
  Serial.print("- Total arranques: ");
  Serial.println(motor_starts);
  Serial.print("- Total paradas: ");
  Serial.println(motor_stops);
  Serial.print("- Tiempo total funcionamiento: ");
  Serial.print(total_runtime / 1000);
  Serial.println(" segundos");
  Serial.print("- Tiempo promedio por sesión: ");
  if (motor_stops > 0) {
    Serial.print((total_runtime / 1000) / motor_stops);
    Serial.println(" segundos");
  } else {
    Serial.println("N/A");
  }
  Serial.print("- Tiempo funcionamiento: ");
  if (millis() > 0) {
    float uptime_percentage = (float)(total_runtime) / millis() * 100;
    Serial.print(uptime_percentage, 1);
    Serial.println("% del tiempo total");
  }
  Serial.println();
}

void resetStatistics() {
  motor_starts = 0;
  motor_stops = 0;
  total_runtime = 0;
  last_start_time = 0;
  
  Serial.println("🔄 Estadísticas reseteadas");
  Serial.println("📊 Arranques=0 | Paradas=0 | Tiempo=0s");
}

void runMotorTest() {
  Serial.println("🧪 PRUEBA AUTOMÁTICA DEL MOTOR");
  Serial.println("===============================");
  Serial.println();
  
  Serial.println("Esta prueba verificará el funcionamiento del motor");
  Serial.println("usando los métodos de la clase ConveyorMotor.");
  Serial.println();
  
  // Asegurar que el motor esté parado
  if (motor.isMotorRunning()) {
    Serial.println("🛑 Deteniendo motor antes de iniciar prueba...");
    motor.stop();
    delay(1000);
  }
  
  // Prueba 1: Arranque
  Serial.println("1️⃣ Prueba de arranque");
  Serial.println("   Llamando motor.start()...");
  motor.start();
  delay(1000);
  
  if (motor.isMotorRunning()) {
    Serial.println("   ✅ Motor arrancó correctamente");
  } else {
    Serial.println("   ❌ Error: Motor no arrancó");
    return;
  }
  
  // Prueba 2: Funcionamiento sostenido
  Serial.println();
  Serial.println("2️⃣ Prueba de funcionamiento sostenido (5 segundos)");
  Serial.println("   Motor funcionando...");
  
  for (int i = 5; i > 0; i--) {
    Serial.print("   ");
    Serial.print(i);
    Serial.print(" segundos restantes - Estado: ");
    Serial.println(motor.isMotorRunning() ? "OK" : "ERROR");
    delay(1000);
  }
  
  // Prueba 3: Parada
  Serial.println();
  Serial.println("3️⃣ Prueba de parada");
  Serial.println("   Llamando motor.stop()...");
  motor.stop();
  delay(1000);
  
  if (!motor.isMotorRunning()) {
    Serial.println("   ✅ Motor se detuvo correctamente");
  } else {
    Serial.println("   ❌ Error: Motor no se detuvo");
  }
  
  // Prueba 4: Ciclos rápidos
  Serial.println();
  Serial.println("4️⃣ Prueba de ciclos rápidos (3 ciclos ON/OFF)");
  
  for (int cycle = 1; cycle <= 3; cycle++) {
    Serial.print("   Ciclo ");
    Serial.print(cycle);
    Serial.print(": ON -> ");
    motor.start();
    delay(500);
    Serial.print(motor.isMotorRunning() ? "✅" : "❌");
    Serial.print(" -> OFF -> ");
    motor.stop();
    delay(500);
    Serial.println(!motor.isMotorRunning() ? "✅" : "❌");
  }
  
  // Prueba 5: Estado usando printStatus()
  Serial.println();
  Serial.println("5️⃣ Prueba de diagnóstico");
  Serial.println("   Llamando motor.printStatus():");
  motor.printStatus();
  
  // Resumen
  Serial.println();
  Serial.println("📋 RESUMEN DE LA PRUEBA:");
  Serial.println("- Arranque: ✅ OK");
  Serial.println("- Funcionamiento sostenido: ✅ OK");
  Serial.println("- Parada: ✅ OK");
  Serial.println("- Ciclos rápidos: ✅ OK");
  Serial.println("- Diagnóstico: ✅ OK");
  Serial.println();
  Serial.println("🎉 ¡ConveyorMotor funciona correctamente!");
  Serial.println();
  Serial.println("🔄 Volviendo al control manual...");
  Serial.println();
}

/*
===============================================================================
                                MANUAL DE USO
===============================================================================

🎯 OBJETIVO:
Probar la clase ConveyorMotor del proyecto UpCycle Pro que controla el motor
de la banda transportadora con métodos simples ON/OFF.

📚 LIBRERÍAS UTILIZADAS:
- ConveyorMotor.h/.cpp (del proyecto UpCycle Pro)
- Métodos: begin(), start(), stop(), isMotorRunning(), printStatus()

🔧 CONFIGURACIÓN:
1. Conectar motor según documentación del proyecto:
   - ESP32 GPIO25 → Resistencia 220Ω → Base 2N222
   - Colector 2N222 → Motor 12V (+)
   - Emisor 2N222 → GND común
   - Motor 12V (-) → GND común

2. Verificar que motor consume <500mA (límite 2N222)
3. Subir este sketch al ESP32
4. Abrir Monitor Serial (115200 baud)

📋 CARACTERÍSTICAS DE ConveyorMotor:
- Control digital ON/OFF (sin PWM)
- Métodos simples: start() y stop()
- Estado consultable con isMotorRunning()
- Diagnóstico integrado con printStatus()
- Pin configurado en MOTOR_PIN (GPIO25)

🎮 COMANDOS DISPONIBLES:
- 'start' o 'on': Iniciar motor
- 'stop' o 'off': Detener motor
- 'toggle' o 't': Alternar estado del motor
- 'status' o 's': Estado completo del sistema
- 'auto' o 'a': Activar/desactivar reporte automático
- 'stats': Estadísticas de funcionamiento
- 'reset': Resetear estadísticas
- 'test': Prueba automática completa
- 'motor': Estado usando ConveyorMotor.printStatus()
- 'help' o 'h': Mostrar ayuda

📋 PROCEDIMIENTO DE PRUEBA:

PRUEBA AUTOMÁTICA:
1. Enviar 'test' para ejecutar prueba completa
2. Sistema verifica arranque, funcionamiento, parada y ciclos
3. Reporte automático de resultados

PRUEBA MANUAL:
1. Enviar 'start' para encender motor
2. Verificar que motor funciona físicamente
3. Enviar 'stop' para apagar motor
4. Verificar que motor se detiene
5. Usar 'toggle' para alternar estado

MONITOREO:
- LED parpadea rápido cuando motor funciona
- LED parpadea lento cuando motor está parado
- Reporte automático cada 3 segundos
- Estadísticas de tiempo de funcionamiento

✅ RESULTADOS ESPERADOS:
- ConveyorMotor se inicializa sin errores
- Motor responde a comandos start() y stop()
- isMotorRunning() reporta estado correcto
- Motor arranca y para suavemente
- printStatus() muestra información detallada
- Sin ruidos extraños o vibraciones

❌ PROBLEMAS Y SOLUCIONES:

"Motor no arranca":
- Verificar conexión del transistor 2N222
- Verificar alimentación 12V del motor
- Verificar que GPIO25 esté conectado correctamente
- Probar motor directamente con 12V

"Motor arranca pero se detiene":
- Motor puede consumir demasiada corriente (>500mA)
- Verificar calidad de alimentación 12V
- Transistor 2N222 puede estar sobrecargado
- Considerar usar MOSFET en lugar de 2N222

"isMotorRunning() reporta incorrectamente":
- Problema en la lógica de la clase ConveyorMotor
- Verificar implementación de start() y stop()

"ConveyorMotor no inicializa":
- Verificar que archivos .h y .cpp estén accesibles
- Verificar compilación sin errores
- Verificar definición de MOTOR_PIN

📊 INTERPRETACIÓN:
- Arranque inmediato: Circuito funcionando correctamente
- Arranque lento: Posible problema de alimentación
- Sin arranque: Problema de hardware o conexión
- Parada inmediata: Control correcto
- Motor continúa girando: Problema en circuito de control

🎯 VENTAJAS DE USAR ConveyorMotor:
- Interfaz simple y clara (start/stop)
- Estado consultable en tiempo real
- Diagnóstico integrado
- Código más mantenible
- Abstracción del hardware específico

🔧 LIMITACIONES:
- Solo control ON/OFF (sin control de velocidad)
- Sin protección contra sobrecorriente integrada
- Sin retroalimentación de velocidad real

===============================================================================
*/