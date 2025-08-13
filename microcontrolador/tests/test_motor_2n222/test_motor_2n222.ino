/*
  TEST MOTOR CON TRANSISTOR 2N222 - UpCycle Pro
  ==============================================
  
  Este sketch prueba el control del motor de la banda transportadora
  usando un transistor 2N222 para amplificar la señal del ESP32.
  
  Conexiones:
  - ESP32 GPIO25 -> Resistencia 220Ω -> Base del 2N222
  - Colector 2N222 -> Motor 12V (+)
  - Emisor 2N222 -> GND común
  - Motor 12V (-) -> GND común
  - Fuente 12V (+) -> Motor 12V (+)
  
  IMPORTANTE: 
  - Motor debe consumir <500mA para ser compatible con 2N222
  - Verificar polaridad del motor y fuente de alimentación
  
  Funcionamiento:
  - Control PWM del motor (0-100% velocidad)
  - Pruebas de arranque, aceleración y frenado
  - Monitoreo de corriente (simulado)
  - Control manual por comandos seriales
*/

// Definir pin de control del motor
#define MOTOR_PIN 25      // GPIO25 -> Base del 2N222
#define LED_PIN 2         // LED onboard para indicación visual

// Variables de control del motor
int motor_speed = 0;      // Velocidad actual (0-255)
bool motor_running = false;
unsigned long last_change = 0;
unsigned long demo_interval = 2000; // Cambio cada 2 segundos en demo
bool demo_mode = true;
int demo_step = 0;

// Configuración PWM
const int pwm_frequency = 1000;  // 1kHz
const int pwm_channel = 0;
const int pwm_resolution = 8;    // 8 bits (0-255)

void setup() {
  Serial.begin(115200);
  
  // Configurar PWM para el motor
  ledcSetup(pwm_channel, pwm_frequency, pwm_resolution);
  ledcAttachPin(MOTOR_PIN, pwm_channel);
  
  // Configurar LED indicador
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println();
  Serial.println("⚙️ TEST MOTOR 2N222 - UpCycle Pro");
  Serial.println("==================================");
  Serial.println();
  Serial.println("🔧 Conexiones:");
  Serial.println("- ESP32 GPIO25 -> R220Ω -> Base 2N222");
  Serial.println("- Colector 2N222 -> Motor 12V (+)");
  Serial.println("- Emisor 2N222 -> GND común");
  Serial.println("- Motor 12V (-) -> GND común");
  Serial.println();
  
  // Motor inicialmente apagado
  setMotorSpeed(0);
  
  Serial.println("✅ Sistema listo");
  Serial.println();
  Serial.println("🎮 COMANDOS:");
  Serial.println("- '0' = Apagar motor");
  Serial.println("- '1' = Velocidad baja (25%)");
  Serial.println("- '2' = Velocidad media (50%)");
  Serial.println("- '3' = Velocidad alta (75%)");
  Serial.println("- '4' = Velocidad máxima (100%)");
  Serial.println("- 'd' = Activar/Desactivar demo automático");
  Serial.println("- 's' = Estado del motor");
  Serial.println("- 't' = Prueba completa del motor");
  Serial.println();
  
  Serial.println("🔄 Iniciando demo automático...");
  Serial.println();
}

void loop() {
  // Procesar comandos seriales
  if (Serial.available()) {
    char command = Serial.read();
    processCommand(command);
  }
  
  // Ejecutar demo automático si está activado
  if (demo_mode && (millis() - last_change > demo_interval)) {
    runDemo();
    last_change = millis();
  }
  
  // Actualizar LED indicador
  updateLED();
  
  delay(50);
}

void processCommand(char cmd) {
  switch(cmd) {
    case '0':
      Serial.println("🛑 Apagando motor");
      setMotorSpeed(0);
      demo_mode = false;
      break;
      
    case '1':
      Serial.println("🐌 Velocidad baja (25%)");
      setMotorSpeed(64); // 25% de 255
      demo_mode = false;
      break;
      
    case '2':
      Serial.println("🚶 Velocidad media (50%)");
      setMotorSpeed(128); // 50% de 255
      demo_mode = false;
      break;
      
    case '3':
      Serial.println("🏃 Velocidad alta (75%)");
      setMotorSpeed(192); // 75% de 255
      demo_mode = false;
      break;
      
    case '4':
      Serial.println("🚀 Velocidad máxima (100%)");
      setMotorSpeed(255); // 100%
      demo_mode = false;
      break;
      
    case 'd':
      demo_mode = !demo_mode;
      Serial.print("🔄 Demo automático: ");
      Serial.println(demo_mode ? "ACTIVADO" : "DESACTIVADO");
      if (demo_mode) {
        demo_step = 0;
        last_change = millis();
      }
      break;
      
    case 's':
      printMotorStatus();
      break;
      
    case 't':
      Serial.println("🧪 Iniciando prueba completa del motor...");
      runCompleteTest();
      break;
      
    default:
      Serial.println("❌ Comando no reconocido");
      Serial.println("💡 Comandos: 0, 1, 2, 3, 4, d, s, t");
      break;
  }
}

void runDemo() {
  switch(demo_step) {
    case 0:
      Serial.println("🔄 Demo: Arrancando motor (25%)");
      setMotorSpeed(64);
      break;
    case 1:
      Serial.println("🔄 Demo: Acelerando (50%)");
      setMotorSpeed(128);
      break;
    case 2:
      Serial.println("🔄 Demo: Velocidad alta (75%)");
      setMotorSpeed(192);
      break;
    case 3:
      Serial.println("🔄 Demo: Velocidad máxima (100%)");
      setMotorSpeed(255);
      break;
    case 4:
      Serial.println("🔄 Demo: Desacelerando (50%)");
      setMotorSpeed(128);
      break;
    case 5:
      Serial.println("🔄 Demo: Parando motor (0%)");
      setMotorSpeed(0);
      break;
  }
  
  demo_step = (demo_step + 1) % 6;
}

void runCompleteTest() {
  demo_mode = false; // Pausar demo durante prueba
  
  Serial.println("📋 SECUENCIA DE PRUEBA:");
  
  // Prueba 1: Arranque suave
  Serial.println("1️⃣ Arranque suave (0% -> 50%)");
  for (int speed = 0; speed <= 128; speed += 8) {
    setMotorSpeed(speed);
    Serial.print("   Velocidad: ");
    Serial.print((speed * 100) / 255);
    Serial.println("%");
    delay(200);
  }
  delay(1000);
  
  // Prueba 2: Aceleración completa
  Serial.println("2️⃣ Aceleración completa (50% -> 100%)");
  for (int speed = 128; speed <= 255; speed += 16) {
    setMotorSpeed(speed);
    Serial.print("   Velocidad: ");
    Serial.print((speed * 100) / 255);
    Serial.println("%");
    delay(300);
  }
  delay(2000); // Mantener velocidad máxima
  
  // Prueba 3: Frenado gradual
  Serial.println("3️⃣ Frenado gradual (100% -> 0%)");
  for (int speed = 255; speed >= 0; speed -= 16) {
    setMotorSpeed(speed);
    Serial.print("   Velocidad: ");
    Serial.print((speed * 100) / 255);
    Serial.println("%");
    delay(200);
  }
  
  // Prueba 4: Arranques rápidos
  Serial.println("4️⃣ Prueba de arranques rápidos");
  for (int i = 0; i < 3; i++) {
    Serial.print("   Arranque rápido ");
    Serial.println(i + 1);
    setMotorSpeed(200);
    delay(1000);
    setMotorSpeed(0);
    delay(500);
  }
  
  Serial.println("✅ Prueba completa finalizada");
  Serial.println("🔄 Reactivando demo automático...");
  demo_mode = true;
  demo_step = 0;
  last_change = millis();
}

void setMotorSpeed(int speed) {
  // Limitar rango
  speed = constrain(speed, 0, 255);
  
  // Aplicar PWM
  ledcWrite(pwm_channel, speed);
  
  // Actualizar variables
  motor_speed = speed;
  motor_running = (speed > 0);
  
  Serial.print("⚙️ Motor: ");
  Serial.print((speed * 100) / 255);
  Serial.print("% (PWM: ");
  Serial.print(speed);
  Serial.println(")");
}

void updateLED() {
  if (motor_running) {
    // LED parpadeando cuando motor está activo
    if ((millis() / 200) % 2) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  } else {
    // LED apagado cuando motor está parado
    digitalWrite(LED_PIN, LOW);
  }
}

void printMotorStatus() {
  Serial.println("📊 ESTADO DEL MOTOR:");
  Serial.print("- Estado: ");
  Serial.println(motor_running ? "FUNCIONANDO ⚙️" : "PARADO 🛑");
  Serial.print("- Velocidad: ");
  Serial.print((motor_speed * 100) / 255);
  Serial.print("% (PWM: ");
  Serial.print(motor_speed);
  Serial.println(")");
  Serial.print("- Demo automático: ");
  Serial.println(demo_mode ? "ACTIVO" : "INACTIVO");
  Serial.print("- Corriente estimada: ");
  Serial.print((motor_speed * 400) / 255); // Simulación: 0-400mA
  Serial.println(" mA");
  Serial.print("- Potencia estimada: ");
  Serial.print((motor_speed * 5) / 255); // Simulación: 0-5W
  Serial.println(" W");
}

/*
===============================================================================
                                MANUAL DE PRUEBA
===============================================================================

🎯 OBJETIVO:
Verificar que el transistor 2N222 controla correctamente el motor 12V.

🔧 CONFIGURACIÓN:
1. Montar circuito según diagrama:
   ESP32 GPIO25 -> Resistencia 220Ω -> Base 2N222
   Colector 2N222 -> Motor 12V (+)
   Emisor 2N222 -> GND común
   Motor (-) -> GND común

2. Conectar fuente 12V para el motor
3. Subir este sketch al ESP32
4. Abrir Monitor Serial (115200 baud)

⚠️ PRECAUCIONES:
- Motor debe consumir <500mA (límite del 2N222)
- Verificar polaridad de motor y fuente
- Usar disipador en 2N222 si motor consume >200mA
- Agregar diodo flyback en paralelo al motor

📋 PROCEDIMIENTO DE PRUEBA:

PRUEBA AUTOMÁTICA:
1. Sistema ejecuta demo automático al inicio
2. Ciclo: 0% -> 25% -> 50% -> 75% -> 100% -> 50% -> 0%
3. LED onboard parpadea cuando motor está activo

PRUEBA MANUAL:
1. Enviar '0' para apagar motor
2. Enviar '1', '2', '3', '4' para velocidades 25%, 50%, 75%, 100%
3. Enviar 't' para prueba completa automática
4. Enviar 's' para ver estado detallado
5. Enviar 'd' para activar/desactivar demo

✅ RESULTADOS ESPERADOS:
- Motor arranca suavemente sin saltos
- Velocidad proporcional al PWM aplicado
- No debe haber ruidos extraños o vibraciones
- LED onboard parpadea cuando motor activo
- Respuesta inmediata a cambios de velocidad

❌ PROBLEMAS COMUNES:
- "Motor no gira":
  * Verificar conexiones del transistor
  * Verificar alimentación 12V del motor
  * Probar motor directamente con 12V
  * Verificar que el motor no esté bloqueado
  
- "Motor gira muy lento o errático":
  * Verificar que el 2N222 no esté saturado
  * Reducir corriente del motor o usar transistor más potente
  * Verificar resistencia de base (220Ω)
  * Verificar calidad de alimentación 12V
  
- "Transistor se calienta mucho":
  * Motor consume demasiada corriente (>500mA)
  * Usar transistor Darlington (TIP120) o MOSFET
  * Agregar disipador térmico
  
- "Ruido o interferencia":
  * Agregar diodo flyback (1N4007) en paralelo al motor
  * Agregar capacitor de filtro en alimentación 12V
  * Verificar conexiones de GND común

📊 INTERPRETACIÓN:
- Arranque suave: Circuito funcionando correctamente
- Velocidad errática: Problemas de alimentación o transistor
- Calentamiento: Exceso de corriente o transistor inadecuado
- Ruido: Falta de protección contra picos inductivos

🔧 MEJORAS POSIBLES:
- Reemplazar 2N222 por MOSFET para motores más potentes
- Agregar sensor de corriente (ACS712) para monitoreo real
- Implementar control PID para velocidad constante
- Agregar protecciones (fusible, diodo flyback)

===============================================================================
*/