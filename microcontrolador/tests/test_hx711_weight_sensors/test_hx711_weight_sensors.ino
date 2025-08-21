/*
  TEST HX711 WEIGHT SENSORS - UpCycle Pro
  =======================================
  
  Este sketch prueba los 3 módulos HX711 con celdas de carga de 50kg
  para la validación final de peso de cada material clasificado.
  
  Hardware necesario:
  - 3 módulos HX711
  - 3 celdas de carga de 50kg
  - ESP32
  
  Conexiones:
  HX711 #1 (VIDRIO):    DOUT -> GPIO32, SCK -> GPIO33
  HX711 #2 (PLASTICO):  DOUT -> GPIO16, SCK -> GPIO17  
  HX711 #3 (METAL):     DOUT -> GPIO18, SCK -> GPIO19
  
  Alimentación: 5V para HX711, GND común
  
  Funcionalidad:
  - Lectura simultánea de los 3 sensores
  - Calibración individual de cada celda
  - Validación de peso por tipo de material
  - Detección automática de objetos
  - Predicción de material por peso
*/

#include "HX711.h"

// Incluir librerías del proyecto
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/WeightManager.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/WeightManager.cpp"

// ===============================================================================
// OBJETOS GLOBALES
// ===============================================================================

WeightManager weightSensors;

// Variables para test automático
unsigned long lastTestUpdate = 0;
const unsigned long TEST_INTERVAL = 2000; // 2 segundos
bool autoTestRunning = false;
int testStep = 0;

// ===============================================================================
// SETUP
// ===============================================================================

void setup() {
  Serial.begin(115200);
  
  Serial.println();
  Serial.println("⚖️ TEST HX711 WEIGHT SENSORS - UpCycle Pro");
  Serial.println("===========================================");
  Serial.println();
  
  Serial.println("📋 Configuración de hardware:");
  Serial.println("   HX711 #1 (VIDRIO):   DOUT=GPIO32, SCK=GPIO33");
  Serial.println("   HX711 #2 (PLASTICO): DOUT=GPIO16, SCK=GPIO17");
  Serial.println("   HX711 #3 (METAL):    DOUT=GPIO18, SCK=GPIO19");
  Serial.println();
  
  // Inicializar sistema de peso
  if (!weightSensors.begin()) {
    Serial.println("❌ Error inicializando celdas de carga");
    Serial.println("🔄 Verificar conexiones y reiniciar");
    while(1) delay(1000);
  }
  
  Serial.println("✅ Sistema de peso inicializado correctamente");
  Serial.println();
  
  printTestInstructions();
  
  // Iniciar test automático
  autoTestRunning = true;
  lastTestUpdate = millis();
  
  Serial.println("🔄 Iniciando monitoreo automático...");
  Serial.println();
}

// ===============================================================================
// LOOP PRINCIPAL
// ===============================================================================

void loop() {
  // Procesar comandos seriales
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    command.toLowerCase();
    processCommand(command);
  }
  
  // Test automático
  if (autoTestRunning && (millis() - lastTestUpdate > TEST_INTERVAL)) {
    runAutomaticTest();
    lastTestUpdate = millis();
  }
  
  delay(100);
}

// ===============================================================================
// PROCESAMIENTO DE COMANDOS
// ===============================================================================

void processCommand(String command) {
  Serial.println("📨 Comando recibido: '" + command + "'");
  
  if (command == "weights" || command == "w") {
    weightSensors.printWeights();
    
  } else if (command == "status" || command == "s") {
    weightSensors.printStatus();
    
  } else if (command == "tare" || command == "t") {
    Serial.println("⚖️ Realizando tara de todas las celdas...");
    weightSensors.tareAll();
    
  } else if (command == "tare1") {
    Serial.println("⚖️ Realizando tara de celda 1 (VIDRIO)...");
    weightSensors.tareScale1();
    
  } else if (command == "tare2") {
    Serial.println("⚖️ Realizando tara de celda 2 (PLASTICO)...");
    weightSensors.tareScale2();
    
  } else if (command == "tare3") {
    Serial.println("⚖️ Realizando tara de celda 3 (METAL)...");
    weightSensors.tareScale3();
    
  } else if (command == "cal1") {
    Serial.println("🔧 Iniciando calibración de celda 1 (VIDRIO)");
    Serial.println("📦 Ingresa el peso conocido en kg (ejemplo: 1.0):");
    while(!Serial.available()) delay(100);
    float knownWeight = Serial.parseFloat();
    Serial.readString(); // Limpiar buffer
    weightSensors.calibrateScale1(knownWeight);
    
  } else if (command == "cal2") {
    Serial.println("🔧 Iniciando calibración de celda 2 (PLASTICO)");
    Serial.println("📦 Ingresa el peso conocido en kg (ejemplo: 1.0):");
    while(!Serial.available()) delay(100);
    float knownWeight = Serial.parseFloat();
    Serial.readString(); // Limpiar buffer
    weightSensors.calibrateScale2(knownWeight);
    
  } else if (command == "cal3") {
    Serial.println("🔧 Iniciando calibración de celda 3 (METAL)");
    Serial.println("📦 Ingresa el peso conocido en kg (ejemplo: 1.0):");
    while(!Serial.available()) delay(100);
    float knownWeight = Serial.parseFloat();
    Serial.readString(); // Limpiar buffer
    weightSensors.calibrateScale3(knownWeight);
    
  } else if (command == "calinfo") {
    weightSensors.printCalibrationInfo();
    
  } else if (command == "auto") {
    autoTestRunning = !autoTestRunning;
    Serial.print("🔄 Test automático: ");
    Serial.println(autoTestRunning ? "ACTIVADO" : "DESACTIVADO");
    
  } else if (command == "detect") {
    runDetectionTest();
    
  } else if (command == "predict") {
    runMaterialPrediction();
    
  } else if (command == "test") {
    runCompleteTest();
    
  } else if (command == "help") {
    printTestInstructions();
    
  } else {
    Serial.println("❌ Comando no reconocido: '" + command + "'");
    Serial.println("💡 Escribe 'help' para ver comandos disponibles");
  }
  
  Serial.println();
}

// ===============================================================================
// FUNCIONES DE TEST
// ===============================================================================

void runAutomaticTest() {
  float weight1, weight2, weight3;
  weightSensors.readAllWeights(weight1, weight2, weight3);
  
  // Mostrar pesos solo si hay cambios significativos
  static float lastW1 = 0, lastW2 = 0, lastW3 = 0;
  bool hasChange = (abs(weight1 - lastW1) > 0.01f || 
                   abs(weight2 - lastW2) > 0.01f || 
                   abs(weight3 - lastW3) > 0.01f);
  
  if (hasChange || testStep % 5 == 0) { // Mostrar cada 10 segundos aunque no haya cambios
    Serial.print("📊 Pesos: ");
    Serial.print("V:" + String(weight1, 2) + "kg ");
    Serial.print("P:" + String(weight2, 2) + "kg ");
    Serial.print("M:" + String(weight3, 2) + "kg");
    
    // Detectar objetos
    if (weightSensors.hasAnyObject()) {
      Serial.print(" | 📦 Objetos: ");
      if (weightSensors.hasObjectOnScale1()) Serial.print("V ");
      if (weightSensors.hasObjectOnScale2()) Serial.print("P ");
      if (weightSensors.hasObjectOnScale3()) Serial.print("M ");
      
      // Predicción de material
      int materialPredicted = weightSensors.getMaterialByWeight(weight1, weight2, weight3);
      if (materialPredicted > 0) {
        String materials[] = {"", "VIDRIO", "PLASTICO", "METAL"};
        Serial.print("| 🎯 Predicción: " + materials[materialPredicted]);
      }
    }
    
    Serial.println();
    
    lastW1 = weight1;
    lastW2 = weight2;
    lastW3 = weight3;
  }
  
  testStep++;
}

void runDetectionTest() {
  Serial.println("🔍 EJECUTANDO TEST DE DETECCIÓN");
  Serial.println("===============================");
  
  for (int i = 0; i < 10; i++) {
    float w1, w2, w3;
    weightSensors.readAllWeights(w1, w2, w3);
    
    Serial.print("Iteración " + String(i + 1) + ": ");
    Serial.print("Pesos=[" + String(w1, 3) + ", " + String(w2, 3) + ", " + String(w3, 3) + "] ");
    Serial.print("Objetos=[");
    Serial.print(weightSensors.hasObjectOnScale1() ? "V" : "-");
    Serial.print(weightSensors.hasObjectOnScale2() ? "P" : "-");  
    Serial.print(weightSensors.hasObjectOnScale3() ? "M" : "-");
    Serial.println("]");
    
    delay(500);
  }
  
  Serial.println("✅ Test de detección completado");
}

void runMaterialPrediction() {
  Serial.println("🎯 EJECUTANDO TEST DE PREDICCIÓN");
  Serial.println("================================");
  
  float w1, w2, w3;
  weightSensors.readAllWeights(w1, w2, w3);
  
  Serial.println("📊 Pesos actuales:");
  Serial.println("   Celda 1 (VIDRIO): " + String(w1, 3) + " kg");
  Serial.println("   Celda 2 (PLASTICO): " + String(w2, 3) + " kg");
  Serial.println("   Celda 3 (METAL): " + String(w3, 3) + " kg");
  Serial.println();
  
  // Validación individual por material
  Serial.println("✅ Validación por material:");
  Serial.println("   Celda 1 - Rango VIDRIO: " + String(weightSensors.isValidVidrioWeight(w1) ? "VÁLIDO" : "INVÁLIDO"));
  Serial.println("   Celda 2 - Rango PLASTICO: " + String(weightSensors.isValidPlasticoWeight(w2) ? "VÁLIDO" : "INVÁLIDO"));
  Serial.println("   Celda 3 - Rango METAL: " + String(weightSensors.isValidMetalWeight(w3) ? "VÁLIDO" : "INVÁLIDO"));
  Serial.println();
  
  // Predicción automática
  int materialPredicted = weightSensors.getMaterialByWeight(w1, w2, w3);
  String materials[] = {"NINGUNO", "VIDRIO", "PLASTICO", "METAL"};
  
  Serial.println("🎯 Predicción final: " + materials[materialPredicted]);
  
  if (materialPredicted > 0) {
    Serial.println("✅ Material detectado correctamente");
  } else {
    Serial.println("❌ No se detectó material válido");
  }
}

void runCompleteTest() {
  Serial.println("🧪 EJECUTANDO TEST COMPLETO");
  Serial.println("===========================");
  
  // Test 1: Verificar estado de hardware
  Serial.println("1️⃣ Verificando estado de hardware...");
  weightSensors.printStatus();
  delay(2000);
  
  // Test 2: Test de detección
  Serial.println("2️⃣ Test de detección de objetos...");
  runDetectionTest();
  delay(2000);
  
  // Test 3: Test de predicción
  Serial.println("3️⃣ Test de predicción de materiales...");
  runMaterialPrediction();
  delay(2000);
  
  // Test 4: Información de calibración
  Serial.println("4️⃣ Información de calibración...");
  weightSensors.printCalibrationInfo();
  delay(2000);
  
  Serial.println("✅ TEST COMPLETO FINALIZADO");
  Serial.println("🔄 Regresando al monitoreo automático...");
}

void printTestInstructions() {
  Serial.println("📚 INSTRUCCIONES DE PRUEBA:");
  Serial.println("============================");
  Serial.println("COMANDOS BÁSICOS:");
  Serial.println("  weights, w    → Mostrar pesos actuales");
  Serial.println("  status, s     → Estado completo del sistema");
  Serial.println("  tare, t       → Tara de todas las celdas");
  Serial.println("  tare1/2/3     → Tara individual de cada celda");
  Serial.println();
  Serial.println("COMANDOS DE CALIBRACIÓN:");
  Serial.println("  cal1/2/3      → Calibrar celda individual");
  Serial.println("  calinfo       → Información de calibración");
  Serial.println();
  Serial.println("COMANDOS DE TEST:");
  Serial.println("  detect        → Test de detección de objetos");
  Serial.println("  predict       → Test de predicción de materiales");
  Serial.println("  test          → Test completo del sistema");
  Serial.println("  auto          → Activar/desactivar monitoreo automático");
  Serial.println("  help          → Mostrar esta ayuda");
  Serial.println();
  Serial.println("CÓMO USAR:");
  Serial.println("1. Verifica que todas las celdas estén conectadas");
  Serial.println("2. Realiza tara con comando 'tare'");
  Serial.println("3. Coloca objetos en las celdas para probar");
  Serial.println("4. Observa la predicción automática de materiales");
  Serial.println("5. Calibra si los pesos no son precisos (cal1/2/3)");
  Serial.println();
  Serial.println("⚠️  RANGOS DE PESO POR MATERIAL:");
  Serial.println("   VIDRIO:   0.1kg - 15.0kg");
  Serial.println("   PLASTICO: 0.05kg - 5.0kg");
  Serial.println("   METAL:    0.2kg - 30.0kg");
  Serial.println();
}

/*
===============================================================================
                                MANUAL DE USO
===============================================================================

🎯 OBJETIVO:
Probar el sistema de pesaje con 3 celdas de carga HX711 para validación
final de materiales clasificados por peso.

🔧 HARDWARE NECESARIO:
- ESP32
- 3 módulos HX711
- 3 celdas de carga de 50kg
- Fuente de alimentación 5V
- Cables dupont y breadboard

📋 CONEXIONES:
HX711 #1 (VIDRIO):
- VDD → 5V, GND → GND
- DT (DOUT) → GPIO32
- SCK → GPIO33

HX711 #2 (PLASTICO):
- VDD → 5V, GND → GND  
- DT (DOUT) → GPIO16
- SCK → GPIO17

HX711 #3 (METAL):
- VDD → 5V, GND → GND
- DT (DOUT) → GPIO18
- SCK → GPIO19

CELDAS DE CARGA:
- Rojo → E+, Negro → E-
- Blanco → A-, Verde → A+

🧪 PROCEDIMIENTO DE PRUEBA:

FASE 1 - INICIALIZACIÓN:
1. Subir sketch al ESP32
2. Abrir Monitor Serial (115200 baud)
3. Verificar que las 3 celdas se inicialicen correctamente
4. Debe aparecer "Sistema de peso inicializado correctamente"

FASE 2 - CALIBRACIÓN (OPCIONAL):
1. Usar comando 'tare' para hacer tara de todas las celdas
2. Para calibración precisa usar 'cal1', 'cal2', 'cal3'
3. Seguir instrucciones para colocar peso conocido
4. Verificar calibración con 'calinfo'

FASE 3 - PRUEBA BÁSICA:
1. Colocar objetos en cada celda
2. Usar comando 'weights' para ver pesos
3. Verificar que detecte objetos con 'status'
4. Observar predicción automática de materiales

FASE 4 - PRUEBAS AVANZADAS:
1. Comando 'detect' → Test de detección
2. Comando 'predict' → Test de predicción
3. Comando 'test' → Test completo automático

FASE 5 - MONITOREO EN TIEMPO REAL:
1. El sistema muestra automáticamente pesos cada 2 segundos
2. Indica cuando detecta objetos
3. Predice tipo de material basado en peso y celda

✅ RESULTADOS ESPERADOS:

INICIALIZACIÓN:
- Las 3 celdas HX711 responden correctamente
- Tara automática se ejecuta sin errores
- Factores de escala predeterminados se cargan

LECTURA DE PESOS:
- Pesos estables sin fluctuaciones excesivas
- Respuesta inmediata a colocación/remoción de objetos
- Precisión de ±10g en el rango 0-50kg

DETECCIÓN DE OBJETOS:
- Detecta objetos >50g correctamente
- No detecta falsos positivos por vibración
- Responde rápidamente a cambios

PREDICCIÓN DE MATERIALES:
- VIDRIO: Objetos 0.1-15kg en celda 1
- PLASTICO: Objetos 0.05-5kg en celda 2
- METAL: Objetos 0.2-30kg en celda 3

❌ PROBLEMAS COMUNES:

"Error: Celda de carga X no responde":
- Verificar conexiones DOUT y SCK
- Verificar alimentación 5V del HX711
- Probar con otro módulo HX711
- Verificar continuidad de cables

"Lecturas inestables o erráticas":
- Verificar conexiones de la celda de carga
- Asegurar que la celda esté bien montada
- Evitar vibraciones mecánicas
- Verificar interferencia electromagnética

"Pesos incorrectos":
- Realizar calibración con peso conocido
- Verificar que la celda no esté sobrecargada
- Hacer tara correctamente antes de usar
- Verificar que no haya offset mecánico

"No detecta objetos livianos":
- Ajustar MIN_WEIGHT en WeightManager.h
- Verificar sensibilidad de la celda
- Realizar tara más precisa
- Verificar ruido eléctrico

"Predicción incorrecta de materiales":
- Ajustar rangos de peso en WeightManager.h
- Verificar que el objeto esté en la celda correcta
- Considerar propiedades específicas de materiales locales

📊 PARÁMETROS AJUSTABLES:

En WeightManager.h se pueden ajustar:
- MIN_WEIGHT: Peso mínimo detectado
- Rangos de peso por material (MIN/MAX)
- Factores de calibración predeterminados
- Número de muestras para filtrado
- Timeout de estabilización

🚀 INTEGRACIÓN:
Una vez probado correctamente, integrar con SimpleClassifier
para validación final de peso después de clasificación por PIR.

===============================================================================
*/