/*
  TEST PIR SENSORS - UpCycle Pro
  ================================
  
  Este sketch prueba individualmente los 3 sensores PIR
  utilizados en el sistema de clasificación.
  
  Conexiones:
  - PIR1 (VIDRIO):  GPIO26
  - PIR2 (PLASTICO): GPIO27  
  - PIR3 (METAL):   GPIO14
  - LED onboard:    GPIO2 (indicador visual)
  
  Funcionamiento:
  - Muestra estado de cada PIR en Monitor Serial
  - LED onboard parpadea cuando cualquier PIR detecta movimiento
  - Contador de detecciones por sensor
  - Timing de detecciones
*/

// Definir pines de sensores PIR
#define PIR1_PIN 26  // Sensor para VIDRIO
#define PIR2_PIN 27  // Sensor para PLASTICO
#define PIR3_PIN 14  // Sensor para METAL
#define LED_PIN 2    // LED onboard para indicación visual

// Variables para el estado de los sensores
bool pir1_state = false;
bool pir2_state = false;
bool pir3_state = false;

// Variables para detección de cambios
bool pir1_last = false;
bool pir2_last = false;
bool pir3_last = false;

// Contadores de detecciones
unsigned long pir1_count = 0;
unsigned long pir2_count = 0;
unsigned long pir3_count = 0;

// Timing
unsigned long last_report = 0;
unsigned long led_off_time = 0;
bool led_state = false;

void setup() {
  Serial.begin(115200);
  
  // Configurar pines
  pinMode(PIR1_PIN, INPUT);
  pinMode(PIR2_PIN, INPUT);
  pinMode(PIR3_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // LED inicial apagado
  digitalWrite(LED_PIN, LOW);
  
  // Mensaje de inicio
  Serial.println();
  Serial.println("🔍 TEST PIR SENSORS - UpCycle Pro");
  Serial.println("==================================");
  Serial.println();
  Serial.println("Conexiones:");
  Serial.println("- PIR1 (VIDRIO):  GPIO26");
  Serial.println("- PIR2 (PLASTICO): GPIO27");
  Serial.println("- PIR3 (METAL):   GPIO14");
  Serial.println("- LED onboard:    GPIO2");
  Serial.println();
  
  // Tiempo de estabilización
  Serial.println("🔄 Esperando estabilización de sensores (5 segundos)...");
  delay(5000);
  
  Serial.println("✅ Sistema listo - Iniciando monitoreo");
  Serial.println("📊 Formato: [PIR1|PIR2|PIR3] - Contador: [C1|C2|C3]");
  Serial.println();
}

void loop() {
  // Leer estado actual de sensores
  pir1_state = digitalRead(PIR1_PIN);
  pir2_state = digitalRead(PIR2_PIN);
  pir3_state = digitalRead(PIR3_PIN);
  
  // Detectar transiciones de LOW a HIGH (nueva detección)
  if (pir1_state && !pir1_last) {
    pir1_count++;
    activateLED();
    Serial.println("🔵 PIR1 (VIDRIO) - DETECCIÓN!");
  }
  
  if (pir2_state && !pir2_last) {
    pir2_count++;
    activateLED();
    Serial.println("🟠 PIR2 (PLASTICO) - DETECCIÓN!");
  }
  
  if (pir3_state && !pir3_last) {
    pir3_count++;
    activateLED();
    Serial.println("⚫ PIR3 (METAL) - DETECCIÓN!");
  }
  
  // Actualizar estados anteriores
  pir1_last = pir1_state;
  pir2_last = pir2_state;
  pir3_last = pir3_state;
  
  // Apagar LED después de 1 segundo
  if (led_state && millis() > led_off_time) {
    digitalWrite(LED_PIN, LOW);
    led_state = false;
  }
  
  // Reporte periódico cada 3 segundos
  if (millis() - last_report > 3000) {
    printStatus();
    last_report = millis();
  }
  
  delay(50); // Pequeña pausa para evitar lecturas excesivas
}

void activateLED() {
  digitalWrite(LED_PIN, HIGH);
  led_state = true;
  led_off_time = millis() + 1000; // LED encendido por 1 segundo
}

void printStatus() {
  Serial.print("📊 Estado: [");
  Serial.print(pir1_state ? "🟢" : "🔴");
  Serial.print("|");
  Serial.print(pir2_state ? "🟢" : "🔴");
  Serial.print("|");
  Serial.print(pir3_state ? "🟢" : "🔴");
  Serial.print("] - Contador: [");
  Serial.print(pir1_count);
  Serial.print("|");
  Serial.print(pir2_count);
  Serial.print("|");
  Serial.print(pir3_count);
  Serial.print("] - Tiempo: ");
  Serial.print(millis() / 1000);
  Serial.println("s");
}

/*
===============================================================================
                                MANUAL DE PRUEBA
===============================================================================

🎯 OBJETIVO:
Verificar que los 3 sensores PIR detectan correctamente el movimiento.

🔧 CONFIGURACIÓN:
1. Conectar los sensores PIR según el diagrama
2. Subir este sketch al ESP32
3. Abrir Monitor Serial (115200 baud)

📋 PROCEDIMIENTO DE PRUEBA:
1. Esperar mensaje "Sistema listo"
2. Mover la mano frente a PIR1 (GPIO26)
   - Debe aparecer "🔵 PIR1 (VIDRIO) - DETECCIÓN!"
   - LED onboard debe parpadear
   - Contador PIR1 debe incrementar
3. Repetir para PIR2 (GPIO27) y PIR3 (GPIO14)
4. Observar reporte periódico cada 3 segundos

✅ RESULTADOS ESPERADOS:
- Cada sensor debe detectar movimiento independientemente
- LED onboard parpadea con cada detección
- Contadores incrementan correctamente
- No debe haber detecciones falsas en reposo

❌ PROBLEMAS COMUNES:
- "PIR no detecta": Verificar alimentación 5V y conexión de señal
- "Detecciones falsas": Esperar estabilización completa (30-60 seg)
- "LED no parpadea": Verificar conexión GPIO2

📊 INTERPRETACIÓN:
- 🟢: Sensor detecta movimiento
- 🔴: Sensor en reposo
- Contadores muestran total de detecciones desde inicio

===============================================================================
*/