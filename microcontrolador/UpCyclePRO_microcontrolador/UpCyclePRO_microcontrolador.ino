/*
  CLASIFICADORA DE MATERIALES - VERSIÓN SIMPLE CON OLED
  ======================================================
  
  Lógica simple:
  - Recibe número 1, 2 o 3 por Serial
  - 1 = VIDRIO  → Activa PIR1 → Servo1
  - 2 = PLASTICO → Activa PIR2 → Servo2  
  - 3 = METAL   → Activa PIR3 → Servo3
  
  Display OLED:
  - Pantalla principal: Ícono de reciclaje ♻️
  - Al seleccionar modo: Muestra número grande + material
  - Esperando: "ESPERANDO: VIDRIO/PLASTICO/METAL"
  - Procesando: "PROCESANDO: VIDRIO/PLASTICO/METAL"
  
  Flujo:
  1. Enviar número (1, 2 o 3)
  2. OLED muestra modo seleccionado
  3. Motor se enciende, OLED muestra "ESPERANDO"
  4. Sistema espera detección del PIR correspondiente
  5. Cuando PIR detecta → Motor se apaga → OLED "PROCESANDO" → Servo empuja
  6. OLED "COMPLETADO" → Vuelve a "ESPERANDO" + motor reinicia
  
  Conexiones:
  - Motor: GPIO25 → Resistencia 220Ω → Base 2N2222
  - PIR1: GPIO26 (VIDRIO)
  - PIR2: GPIO27 (PLASTICO)
  - PIR3: GPIO14 (METAL)
  - PCA9685: SDA=GPIO21, SCL=GPIO22
    - Canal 0: Servo VIDRIO
    - Canal 1: Servo PLASTICO
    - Canal 2: Servo METAL
  - OLED: SDA=GPIO21, SCL=GPIO22 (compartido con PCA9685)
*/

#include "SimpleClassifier.h"

// Crear instancia del sistema
SimpleClassifier classifier;

void setup() {
  // Inicializar sistema
  if (!classifier.begin()) {
    Serial.println("💥 ERROR CRÍTICO: Sistema no inicializado");
    while(1) {
      delay(1000);
    }
  }
  
  Serial.println("🚀 Sistema listo - Envía 1, 2 o 3 para clasificar");
  Serial.println("📺 Display OLED muestra ícono de reciclaje");
}

void loop() {
  // Actualizar sistema (verificar sensores + OLED)
  classifier.update();
  
  // Procesar comandos seriales
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    
    Serial.println("📨 Comando recibido: " + command);
    
    if (command == "1") {
      Serial.println("🔵 Modo VIDRIO seleccionado");
      classifier.setMode(1);
    }
    else if (command == "2") {
      Serial.println("🟠 Modo PLASTICO seleccionado");
      classifier.setMode(2);
    }
    else if (command == "3") {
      Serial.println("⚫ Modo METAL seleccionado");
      classifier.setMode(3);
    }
    else if (command == "stop") {
      Serial.println("🛑 Deteniendo sistema");
      classifier.stopSystem();
    }
    else if (command == "status") {
      classifier.printStatus();
    }
    else if (command == "help") {
      classifier.printHelp();
    }
    else {
      Serial.println("❌ Comando no reconocido: " + command);
      Serial.println("💡 Comandos válidos: 1, 2, 3, stop, status, help");
    }
  }
  
  delay(100);
}

/*
===============================================================================
                                  MANUAL DE USO
===============================================================================

🎮 COMANDOS:
- "1" → Clasificar VIDRIO
- "2" → Clasificar PLASTICO  
- "3" → Clasificar METAL
- "stop" → Detener sistema
- "status" → Ver estado
- "help" → Mostrar ayuda

📺 PANTALLAS OLED:
┌─────────────────┐
│       ♻️        │  ← Pantalla inicial
│   RECICLAJE     │
└─────────────────┘

┌─────────────────┐
│       1         │  ← Al enviar "1"
│    VIDRIO       │
└─────────────────┘

┌─────────────────┐
│  ESPERANDO:     │  ← Motor funcionando
│    VIDRIO       │
└─────────────────┘

┌─────────────────┐
│ PROCESANDO:     │  ← PIR detecta
│    VIDRIO       │
└─────────────────┘

┌─────────────────┐
│    ESTADO:      │  ← Completado
│  COMPLETADO     │
└─────────────────┘

📋 EJEMPLO DE USO COMPLETO:
1. Abrir Monitor Serial
2. OLED muestra ícono ♻️
3. Enviar "1" 
4. OLED muestra "1 VIDRIO" por 2 segundos
5. OLED cambia a "ESPERANDO: VIDRIO"
6. Motor se enciende
7. Colocar objeto de vidrio en la banda
8. Cuando PIR1 detecta:
   - OLED muestra "PROCESANDO: VIDRIO"
   - Motor se apaga
   - Servo1 empuja el vidrio
   - OLED muestra "COMPLETADO" 
9. Motor se vuelve a encender automáticamente
10. OLED vuelve a "ESPERANDO: VIDRIO"
11. Sistema sigue esperando más objetos de vidrio

🔄 CAMBIAR MATERIAL:
- Para cambiar a plástico: enviar "2"
  - OLED mostrará "2 PLASTICO" → "ESPERANDO: PLASTICO"
- Para cambiar a metal: enviar "3"
  - OLED mostrará "3 METAL" → "ESPERANDO: METAL"
- Para volver a vidrio: enviar "1"

⚠️  CONSIDERACIONES:
- Solo detecta el PIR del material seleccionado
- Los otros PIR se ignoran hasta cambiar modo
- Motor se detiene automáticamente al detectar objeto
- Timeout de 30 segundos si no detecta nada
- OLED vuelve al ícono ♻️ después de 3 segundos de inactividad

🔧 CONEXIONES HARDWARE:
===============================================================================

ESP32                    Componente
-----                    ----------

=== MOTOR CON TRANSISTOR 2N2222 ===
GPIO 25             -->  Resistencia 220Ω --> Base 2N2222
                         Colector 2N2222 --> Motor 12V (+)
                         Emisor 2N2222 --> GND común
                         Motor 12V (-) --> GND común

=== SENSORES PIR ===
GPIO 26             -->  PIR1 Sensor (VIDRIO)
GPIO 27             -->  PIR2 Sensor (PLASTICO)  
GPIO 14             -->  PIR3 Sensor (METAL)

=== CONTROLADOR SERVOS PCA9685 + OLED (I2C COMPARTIDO) ===
GPIO 21 (SDA)       -->  SDA del PCA9685 + SDA del OLED
GPIO 22 (SCL)       -->  SCL del PCA9685 + SCL del OLED
                         Canal 0 --> Servo VIDRIO
                         Canal 1 --> Servo PLASTICO
                         Canal 2 --> Servo METAL

=== DISPLAY OLED ===
GPIO 21 (SDA)       -->  SDA del OLED (compartido)
GPIO 22 (SCL)       -->  SCL del OLED (compartido)
VCC                 -->  3.3V (NO 5V)
GND                 -->  GND

=== ALIMENTACIÓN ===
ESP32: 5V
PCA9685: 5V
Servos: 5V (máximo 1.5A total)
PIR Sensores: 5V
Motor: 12V (a través de transistor)
OLED: 3.3V ⚠️ IMPORTANTE

===============================================================================
                                LIBRERÍAS NECESARIAS
===============================================================================

Instalar en Arduino IDE:
1. Adafruit_PWMServoDriver (para PCA9685)
2. Adafruit_SSD1306 (para OLED)
3. Adafruit_GFX (dependencia del OLED)

===============================================================================
                                TROUBLESHOOTING
===============================================================================

❌ "Error inicializando OLED":
- Verificar conexiones I2C (SDA=21, SCL=22)
- Verificar alimentación 3.3V del OLED (NO 5V)
- Verificar dirección I2C (por defecto 0x3C)
- Si no funciona, cambiar a 0x3D en OLEDDisplay.h

❌ "OLED en blanco":
- Verificar que sea 128x32 (no 128x64)
- Si es 128x64, cambiar SCREEN_HEIGHT a 64
- Verificar cables SDA/SCL no invertidos

❌ "Error inicializando servos":
- Verificar que I2C esté compartido correctamente
- Verificar alimentación 5V del PCA9685
- Ambos dispositivos usan I2C pero direcciones diferentes

❌ "PIR no detecta":
- Verificar alimentación 5V de sensores PIR
- Verificar conexiones GPIO 26, 27, 14
- Esperar 2 segundos tras encendido para estabilización

❌ "Motor no gira":
- Verificar consumo del motor (<500mA para 2N2222)
- Verificar conexiones del transistor
- Verificar alimentación 12V del motor

===============================================================================
                                  FLUJO DETALLADO
===============================================================================

ESTADO INICIAL:
- Sistema: IDLE
- Motor: DETENIDO  
- Servos: REPOSO (0°)
- PIRs: Monitoreando pero ignorados
- OLED: Ícono ♻️

COMANDO "1" (VIDRIO):
1. Sistema → WAITING
2. OLED → "1 VIDRIO" (2 segundos)
3. OLED → "ESPERANDO: VIDRIO"
4. Motor → ENCENDIDO
5. Activar monitoreo solo PIR1
6. Ignorar PIR2 y PIR3

DETECCIÓN PIR1:
1. PIR1 detecta movimiento
2. OLED → "PROCESANDO: VIDRIO"
3. Motor → APAGADO
4. Sistema → PROCESSING
5. Servo1 → 90° (empuje) por 1 segundo
6. Servo1 → 0° (reposo)
7. OLED → "COMPLETADO" (1 segundo)
8. Motor → ENCENDIDO
9. OLED → "ESPERANDO: VIDRIO"
10. Sistema → WAITING (listo para siguiente objeto)

TIMEOUT (30 seg sin detección):
1. OLED → "TIMEOUT" (2 segundos)
2. Motor → APAGADO
3. Sistema → IDLE
4. OLED → Ícono ♻️

COMANDO "stop":
1. Motor → APAGADO
2. Todos los servos → REPOSO
3. Sistema → IDLE
4. OLED → Ícono ♻️

===============================================================================
*/