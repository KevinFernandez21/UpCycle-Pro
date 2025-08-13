/*
  TEST PCA9685 SERVO CONTROLLER - UpCycle Pro
  =============================================
  
  Este sketch prueba el controlador PCA9685 y los 3 servos
  utilizados para empujar los materiales clasificados.
  
  Conexiones:
  - PCA9685: SDA=GPIO21, SCL=GPIO22, VCC=5V, GND=GND
  - Servo VIDRIO:   Canal 0 del PCA9685
  - Servo PLASTICO: Canal 1 del PCA9685  
  - Servo METAL:    Canal 2 del PCA9685
  
  Funcionamiento:
  - Inicializa el PCA9685
  - Prueba cada servo individualmente
  - Ciclo completo de prueba automática
  - Control manual por comandos seriales
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Crear objeto del controlador PCA9685
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Definir canales de servos
#define SERVO_VIDRIO 0    // Canal 0 - Servo para VIDRIO
#define SERVO_PLASTICO 1  // Canal 1 - Servo para PLASTICO
#define SERVO_METAL 2     // Canal 2 - Servo para METAL

// Definir posiciones PWM para servos (típicamente 150-600)
#define SERVO_MIN 150     // Posición 0° (reposo)
#define SERVO_MAX 600     // Posición 180° (empuje máximo)
#define SERVO_MID 375     // Posición 90° (empuje normal)

// Variables de control
unsigned long last_test = 0;
int current_test_servo = 0;
bool auto_test = true;
unsigned long test_interval = 3000; // 3 segundos entre pruebas

void setup() {
  Serial.begin(115200);
  
  // Inicializar I2C y PCA9685
  Wire.begin();
  
  Serial.println();
  Serial.println("🎛️ TEST PCA9685 SERVO CONTROLLER - UpCycle Pro");
  Serial.println("===============================================");
  Serial.println();
  
  // Intentar inicializar PCA9685
  Serial.println("🔄 Inicializando PCA9685...");
  pwm.begin();
  pwm.setOscillatorFrequency(27000000); // Frecuencia del oscilador interno
  pwm.setPWMFreq(50); // Frecuencia PWM para servos (50Hz)
  
  delay(1000);
  
  // Verificar comunicación I2C
  Wire.beginTransmission(0x40); // Dirección por defecto del PCA9685
  if (Wire.endTransmission() == 0) {
    Serial.println("✅ PCA9685 detectado correctamente");
  } else {
    Serial.println("❌ ERROR: PCA9685 no detectado");
    Serial.println("   - Verificar conexiones I2C (SDA=21, SCL=22)");
    Serial.println("   - Verificar alimentación 5V");
    while(1) delay(1000);
  }
  
  Serial.println();
  Serial.println("📋 Configuración:");
  Serial.println("- Servo VIDRIO:   Canal 0 (GPIO: empuje hacia contenedor de vidrio)");
  Serial.println("- Servo PLASTICO: Canal 1 (GPIO: empuje hacia contenedor de plástico)");
  Serial.println("- Servo METAL:    Canal 2 (GPIO: empuje hacia contenedor de metal)");
  Serial.println();
  
  // Mover todos los servos a posición de reposo
  Serial.println("🏠 Moviendo servos a posición de reposo...");
  moveServo(SERVO_VIDRIO, SERVO_MIN);
  moveServo(SERVO_PLASTICO, SERVO_MIN);
  moveServo(SERVO_METAL, SERVO_MIN);
  delay(2000);
  
  Serial.println("✅ Sistema listo");
  Serial.println();
  Serial.println("🎮 COMANDOS:");
  Serial.println("- '1' = Probar servo VIDRIO");
  Serial.println("- '2' = Probar servo PLASTICO");
  Serial.println("- '3' = Probar servo METAL");
  Serial.println("- 'a' = Activar/Desactivar prueba automática");
  Serial.println("- 'r' = Reset todos los servos a reposo");
  Serial.println("- 's' = Estado de todos los servos");
  Serial.println();
}

void loop() {
  // Procesar comandos seriales
  if (Serial.available()) {
    char command = Serial.read();
    processCommand(command);
  }
  
  // Ejecutar prueba automática si está activada
  if (auto_test && (millis() - last_test > test_interval)) {
    automaticTest();
    last_test = millis();
  }
  
  delay(100);
}

void processCommand(char cmd) {
  switch(cmd) {
    case '1':
      Serial.println("🔵 Probando servo VIDRIO (Canal 0)");
      testServo(SERVO_VIDRIO, "VIDRIO");
      break;
      
    case '2':
      Serial.println("🟠 Probando servo PLASTICO (Canal 1)");
      testServo(SERVO_PLASTICO, "PLASTICO");
      break;
      
    case '3':
      Serial.println("⚫ Probando servo METAL (Canal 2)");
      testServo(SERVO_METAL, "METAL");
      break;
      
    case 'a':
      auto_test = !auto_test;
      Serial.print("🔄 Prueba automática: ");
      Serial.println(auto_test ? "ACTIVADA" : "DESACTIVADA");
      break;
      
    case 'r':
      Serial.println("🏠 Reset - Todos los servos a posición de reposo");
      resetAllServos();
      break;
      
    case 's':
      printServoStatus();
      break;
      
    default:
      Serial.println("❌ Comando no reconocido");
      Serial.println("💡 Comandos: 1, 2, 3, a, r, s");
      break;
  }
}

void automaticTest() {
  Serial.print("🔄 Prueba automática - Servo ");
  Serial.print(current_test_servo + 1);
  Serial.print(" (");
  
  switch(current_test_servo) {
    case 0:
      Serial.print("VIDRIO");
      testServo(SERVO_VIDRIO, "VIDRIO");
      break;
    case 1:
      Serial.print("PLASTICO");
      testServo(SERVO_PLASTICO, "PLASTICO");
      break;
    case 2:
      Serial.print("METAL");
      testServo(SERVO_METAL, "METAL");
      break;
  }
  Serial.println(")");
  
  current_test_servo = (current_test_servo + 1) % 3;
}

void testServo(int channel, String material) {
  Serial.print("   Reposo (0°) -> ");
  moveServo(channel, SERVO_MIN);
  delay(1000);
  
  Serial.print("Empuje (90°) -> ");
  moveServo(channel, SERVO_MID);
  delay(1500);
  
  Serial.print("Reposo (0°) ✅");
  moveServo(channel, SERVO_MIN);
  Serial.println();
}

void moveServo(int channel, int pulseWidth) {
  pwm.setPWM(channel, 0, pulseWidth);
}

void resetAllServos() {
  moveServo(SERVO_VIDRIO, SERVO_MIN);
  moveServo(SERVO_PLASTICO, SERVO_MIN);
  moveServo(SERVO_METAL, SERVO_MIN);
  Serial.println("✅ Todos los servos en posición de reposo");
}

void printServoStatus() {
  Serial.println("📊 ESTADO DE SERVOS:");
  Serial.println("- Canal 0 (VIDRIO):   Posición de reposo");
  Serial.println("- Canal 1 (PLASTICO): Posición de reposo");
  Serial.println("- Canal 2 (METAL):    Posición de reposo");
  Serial.print("- Prueba automática: ");
  Serial.println(auto_test ? "ACTIVA" : "INACTIVA");
  Serial.print("- Próximo servo a probar: ");
  Serial.println(current_test_servo + 1);
}

/*
===============================================================================
                                MANUAL DE PRUEBA
===============================================================================

🎯 OBJETIVO:
Verificar que el PCA9685 controla correctamente los 3 servos del sistema.

🔧 CONFIGURACIÓN:
1. Conectar PCA9685: SDA=GPIO21, SCL=GPIO22, VCC=5V, GND=GND
2. Conectar servos a canales 0, 1, 2 del PCA9685
3. Alimentar servos con 5V (máximo 1.5A total)
4. Subir este sketch al ESP32
5. Abrir Monitor Serial (115200 baud)

📋 PROCEDIMIENTO DE PRUEBA:

PRUEBA AUTOMÁTICA:
1. Sistema ejecuta ciclo automático cada 3 segundos
2. Prueba servos en secuencia: VIDRIO -> PLASTICO -> METAL
3. Cada servo: reposo -> empuje -> reposo

PRUEBA MANUAL:
1. Enviar '1' para probar servo VIDRIO
2. Enviar '2' para probar servo PLASTICO  
3. Enviar '3' para probar servo METAL
4. Enviar 'a' para activar/desactivar modo automático
5. Enviar 'r' para reset todos los servos
6. Enviar 's' para ver estado

✅ RESULTADOS ESPERADOS:
- PCA9685 detectado al inicio
- Cada servo se mueve suavemente de reposo a empuje y viceversa
- No debe haber jitter o movimientos erráticos
- Respuesta inmediata a comandos manuales

❌ PROBLEMAS COMUNES:
- "PCA9685 no detectado": 
  * Verificar conexiones I2C (SDA=21, SCL=22)
  * Verificar alimentación 5V del PCA9685
  * Probar con scanner I2C para detectar dirección
  
- "Servo no se mueve":
  * Verificar conexión del servo al canal correcto
  * Verificar alimentación 5V de servos (mínimo 1A)
  * Verificar que el servo no esté bloqueado mecánicamente
  
- "Movimiento errático":
  * Verificar calidad de alimentación (usar capacitor)
  * Verificar interferencias en líneas I2C
  * Ajustar valores SERVO_MIN/MAX si es necesario

📊 INTERPRETACIÓN:
- Movimiento suave: Servo funcionando correctamente
- Jitter: Problemas de alimentación o interferencia
- No movimiento: Conexión o alimentación defectuosa

🔧 AJUSTES:
- Modificar SERVO_MIN/MAX si el rango de movimiento no es apropiado
- Cambiar test_interval para pruebas más rápidas/lentas
- Ajustar SERVO_MID para cambiar ángulo de empuje

===============================================================================
*/