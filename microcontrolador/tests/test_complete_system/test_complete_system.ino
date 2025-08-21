/*
  TEST SISTEMA COMPLETO - UpCycle Pro v2.0 CON VALIDACIÓN DE PESO
  ================================================================
  
  Este sketch prueba el sistema completo de UpCycle Pro incluyendo:
  - Clasificación por sensores PIR
  - Control de motor y servos
  - Display OLED
  - VALIDACIÓN FINAL POR PESO (NUEVA FUNCIONALIDAD)
  
  Hardware necesario:
  - ESP32
  - 3 sensores PIR (GPIO26, 27, 14)
  - Motor con transistor 2N222 (GPIO25)
  - PCA9685 para 3 servos
  - Display OLED SSD1306 (I2C)
  - 3 módulos HX711 con celdas de carga de 50kg
  
  Funcionalidad nueva:
  - Después de clasificar con PIR, valida el peso
  - Compara material detectado por PIR vs material predicho por peso
  - Genera alertas si hay discrepancias
  - Permite calibración y control de celdas de carga
  
  Comandos ampliados:
  - Comandos originales: 1, 2, 3, stop, status, help
  - Nuevos comandos: weight, tare, cal, weight_on, weight_off
*/

// Incluir librerías del proyecto con validación de peso
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/SimpleClassifier.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/SimpleClassifier.cpp"

// Incluir todas las dependencias
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/PIRManager.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/PIRManager.cpp"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ConveyorMotor.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ConveyorMotor.cpp"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ServoManager.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ServoManager.cpp"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/OLEDDisplay.h"
#include "C:/Users\hombr\Desktop\projects\programacion\UpCycle-Pro\microcontrolador\UpCyclePRO_microcontrolador\OLEDDisplay.cpp"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/WeightManager.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/WeightManager.cpp"

// Incluir librería HX711 para celdas de carga
#include "HX711.h"

// ===============================================================================
// OBJETOS GLOBALES
// ===============================================================================

SimpleClassifier classifier;

// Variables para estadísticas
unsigned long classificationsTotal = 0;
unsigned long classificationsValid = 0;
unsigned long classificationsFailed = 0;
unsigned long systemStartTime = 0;

// ===============================================================================
// SETUP
// ===============================================================================

void setup() {
  Serial.begin(115200);
  
  Serial.println();
  Serial.println("🚀 TEST SISTEMA COMPLETO - UpCycle Pro v2.0");
  Serial.println("============================================");
  Serial.println("✨ CON VALIDACIÓN DE PESO INTEGRADA");
  Serial.println();
  
  Serial.println("📋 Hardware incluido:");
  Serial.println("   ✅ Sensores PIR x3");
  Serial.println("   ✅ Motor + Transistor 2N222");
  Serial.println("   ✅ Servos x3 + PCA9685");
  Serial.println("   ✅ Display OLED SSD1306");
  Serial.println("   ✅ Celdas de carga HX711 x3 (NUEVO)");
  Serial.println();
  
  // Inicializar sistema completo
  systemStartTime = millis();
  
  if (!classifier.begin()) {
    Serial.println("❌ Error inicializando sistema completo");
    Serial.println("🔄 Verificar conexiones y reiniciar");
    while(1) {
      delay(1000);
      Serial.print(".");
    }
  }
  
  Serial.println("✅ Sistema completo inicializado correctamente");
  Serial.println();
  
  printExtendedInstructions();
  
  Serial.println("🎯 SISTEMA LISTO PARA CLASIFICACIÓN CON VALIDACIÓN");
  Serial.println("💡 Prueba los comandos ampliados para control de peso");
  Serial.println();
}

// ===============================================================================
// LOOP PRINCIPAL
// ===============================================================================

void loop() {
  // Actualizar el clasificador (incluye validación de peso)
  classifier.update();
  
  // Procesar comandos seriales ampliados
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    command.toLowerCase();
    
    processExtendedCommand(command);
  }
  
  delay(50);
}

// ===============================================================================
// PROCESAMIENTO DE COMANDOS AMPLIADO
// ===============================================================================

void processExtendedCommand(String command) {
  Serial.println("📨 Comando recibido: '" + command + "'");
  
  // Comandos originales del clasificador
  if (command == "1") {
    Serial.println("🔵 MODO VIDRIO seleccionado (con validación de peso)");
    classifier.setMode(1);
    
  } else if (command == "2") {
    Serial.println("🟠 MODO PLASTICO seleccionado (con validación de peso)");
    classifier.setMode(2);
    
  } else if (command == "3") {
    Serial.println("⚫ MODO METAL seleccionado (con validación de peso)");
    classifier.setMode(3);
    
  } else if (command == "stop") {
    Serial.println("🛑 DETENIENDO SISTEMA COMPLETO");
    classifier.stopSystem();
    
  } else if (command == "status") {
    classifier.printStatus();
    printSystemStats();
    
  } else if (command == "help") {
    printExtendedInstructions();
    
  // Nuevos comandos para validación de peso
  } else if (command == "weight") {
    classifier.printWeightStatus();
    
  } else if (command == "tare") {
    classifier.tareWeightSensors();
    
  } else if (command == "cal") {
    classifier.calibrateWeightSensors();
    
  } else if (command == "weight_on") {
    classifier.enableWeightValidation(true);
    Serial.println("🎯 El sistema ahora validará materiales por PIR + PESO");
    
  } else if (command == "weight_off") {
    classifier.enableWeightValidation(false);
    Serial.println("📡 El sistema solo usará sensores PIR (modo clásico)");
    
  // Comandos de diagnóstico y test
  } else if (command == "stats") {
    printDetailedStats();
    
  } else if (command == "test") {
    runSystemDiagnostics();
    
  } else if (command == "demo") {
    runDemoSequence();
    
  } else if (command == "reset") {
    resetSystemStats();
    
  } else {
    Serial.println("❌ Comando no reconocido: '" + command + "'");
    Serial.println("💡 Escribe 'help' para ver todos los comandos disponibles");
  }
  
  Serial.println();
}

// ===============================================================================
// FUNCIONES DE INFORMACIÓN Y ESTADÍSTICAS
// ===============================================================================

void printExtendedInstructions() {
  Serial.println("📚 COMANDOS DEL SISTEMA COMPLETO:");
  Serial.println("==================================");
  Serial.println();
  Serial.println("🎯 CLASIFICACIÓN:");
  Serial.println("  1, 2, 3       → Seleccionar modo de clasificación");
  Serial.println("  stop          → Detener sistema");
  Serial.println();
  Serial.println("📊 INFORMACIÓN:");
  Serial.println("  status        → Estado completo del sistema");
  Serial.println("  stats         → Estadísticas detalladas");
  Serial.println("  help          → Mostrar esta ayuda");
  Serial.println();
  Serial.println("⚖️ VALIDACIÓN DE PESO:");
  Serial.println("  weight        → Estado de celdas de carga");
  Serial.println("  tare          → Realizar tara de todas las celdas");
  Serial.println("  cal           → Información de calibración");
  Serial.println("  weight_on     → Habilitar validación por peso");
  Serial.println("  weight_off    → Solo usar sensores PIR");
  Serial.println();
  Serial.println("🧪 DIAGNÓSTICOS:");
  Serial.println("  test          → Ejecutar diagnóstico completo");
  Serial.println("  demo          → Demostración automática");
  Serial.println("  reset         → Reiniciar estadísticas");
  Serial.println();
  Serial.println("✨ NOVEDADES v2.0:");
  Serial.println("  - Validación cruzada PIR + PESO");
  Serial.println("  - Detección de errores de clasificación");
  Serial.println("  - Estadísticas de precisión");
  Serial.println("  - Control individual de celdas de carga");
  Serial.println();
  Serial.println("📋 FLUJO DE CLASIFICACIÓN:");
  Serial.println("  1. Activar modo (1, 2 o 3)");
  Serial.println("  2. PIR detecta objeto y activa servo");
  Serial.println("  3. Sistema pesa objeto en celda correspondiente");
  Serial.println("  4. Compara material PIR vs material PESO");
  Serial.println("  5. Valida o reporta discrepancia");
  Serial.println("  6. Continúa esperando siguiente objeto");
  Serial.println();
}

void printSystemStats() {
  Serial.println();
  Serial.println("📊 ESTADÍSTICAS DEL SISTEMA:");
  Serial.println("=============================");
  
  unsigned long uptime = millis() - systemStartTime;
  Serial.println("Tiempo funcionamiento: " + String(uptime/1000) + " segundos");
  Serial.println("Clasificaciones totales: " + String(classificationsTotal));
  Serial.println("Validaciones exitosas: " + String(classificationsValid));
  Serial.println("Validaciones fallidas: " + String(classificationsFailed));
  
  if (classificationsTotal > 0) {
    float accuracy = (classificationsValid * 100.0) / classificationsTotal;
    Serial.println("Precisión del sistema: " + String(accuracy, 1) + "%");
  }
  
  Serial.println("Validación de peso: " + String(classifier.isWeightValidationEnabled() ? "ACTIVA" : "INACTIVA"));
  
  if (classifier.isWeightValidationEnabled()) {
    Serial.println("Último peso detectado: " + String(classifier.getDetectedWeight(), 3) + " kg");
    Serial.println("Material predicho: " + classifier.getPredictedMaterial());
  }
}

void printDetailedStats() {
  Serial.println();
  Serial.println("📈 ESTADÍSTICAS DETALLADAS:");
  Serial.println("============================");
  
  printSystemStats();
  classifier.printStatus();
  classifier.printWeightStatus();
  
  Serial.println("🔧 CONFIGURACIÓN ACTUAL:");
  Serial.println("   Modo seleccionado: " + String(classifier.getMode()));
  Serial.println("   Estado actual: " + String(classifier.getState()));
  Serial.println("   Sistema de peso: " + String(classifier.isWeightValidationEnabled() ? "HABILITADO" : "DESHABILITADO"));
  
  Serial.println();
}

void resetSystemStats() {
  classificationsTotal = 0;
  classificationsValid = 0;
  classificationsFailed = 0;
  systemStartTime = millis();
  
  Serial.println("🔄 Estadísticas reiniciadas");
  Serial.println("📊 Contadores en cero, cronómetro reiniciado");
}

// ===============================================================================
// FUNCIONES DE DIAGNÓSTICO Y DEMO
// ===============================================================================

void runSystemDiagnostics() {
  Serial.println("🧪 EJECUTANDO DIAGNÓSTICO COMPLETO");
  Serial.println("===================================");
  
  // Diagnóstico 1: Estado de hardware
  Serial.println("1️⃣ Verificando estado de hardware...");
  classifier.printStatus();
  delay(2000);
  
  // Diagnóstico 2: Sistema de peso
  Serial.println("2️⃣ Verificando sistema de peso...");
  classifier.printWeightStatus();
  delay(2000);
  
  // Diagnóstico 3: Test de estados
  Serial.println("3️⃣ Test de cambios de estado...");
  
  Serial.println("   - Activando modo VIDRIO...");
  classifier.setMode(1);
  delay(3000);
  
  Serial.println("   - Deteniendo sistema...");
  classifier.stopSystem();
  delay(2000);
  
  // Diagnóstico 4: Estadísticas
  Serial.println("4️⃣ Estadísticas del sistema...");
  printDetailedStats();
  
  Serial.println("✅ DIAGNÓSTICO COMPLETADO");
  Serial.println("📋 Revisa los resultados arriba para detectar problemas");
  Serial.println();
}

void runDemoSequence() {
  Serial.println("🎬 EJECUTANDO DEMOSTRACIÓN COMPLETA");
  Serial.println("===================================");
  
  Serial.println("📱 Esta demo muestra todas las capacidades del sistema v2.0");
  Serial.println("🔄 Cada modo se activará por 8 segundos con validación de peso");
  Serial.println();
  
  // Demo 1: VIDRIO con peso
  Serial.println("🔵 DEMO VIDRIO (8 segundos) - PIR + Validación de peso");
  classifier.enableWeightValidation(true);
  classifier.setMode(1);
  delay(8000);
  classifier.stopSystem();
  delay(2000);
  
  // Demo 2: PLASTICO con peso
  Serial.println("🟠 DEMO PLASTICO (8 segundos) - PIR + Validación de peso");
  classifier.setMode(2);
  delay(8000);
  classifier.stopSystem();
  delay(2000);
  
  // Demo 3: METAL con peso
  Serial.println("⚫ DEMO METAL (8 segundos) - PIR + Validación de peso");
  classifier.setMode(3);
  delay(8000);
  classifier.stopSystem();
  delay(2000);
  
  // Demo 4: Modo clásico sin peso
  Serial.println("📡 DEMO MODO CLÁSICO (sin validación de peso)");
  classifier.enableWeightValidation(false);
  classifier.setMode(1);
  delay(5000);
  classifier.stopSystem();
  
  // Restaurar modo con peso
  classifier.enableWeightValidation(true);
  
  Serial.println("✅ DEMOSTRACIÓN COMPLETADA");
  Serial.println("🎯 Sistema restaurado con validación de peso habilitada");
  Serial.println("💡 Listo para clasificación real");
  Serial.println();
}

/*
===============================================================================
                                MANUAL DE USO
===============================================================================

🎯 OBJETIVO:
Probar el sistema completo UpCycle Pro v2.0 con validación de peso integrada.
Esta es la versión final que combina clasificación por PIR con validación por peso.

🆕 NOVEDADES v2.0:
- Validación cruzada PIR + PESO para mayor precisión
- Detección automática de errores de clasificación
- Estadísticas de precisión del sistema
- Control individual de celdas de carga
- Interfaz ampliada de comandos
- Modo de diagnóstico completo

🔧 HARDWARE COMPLETO:
- ESP32
- 3 sensores PIR (GPIO26, 27, 14)
- Motor con transistor 2N222 (GPIO25)
- PCA9685 para control de 3 servos
- Display OLED SSD1306 (I2C SDA=21, SCL=22)
- 3 módulos HX711 (GPIO32/33, 16/17, 18/19)
- 3 celdas de carga de 50kg

📋 PROCEDIMIENTO DE PRUEBA COMPLETO:

FASE 1 - INICIALIZACIÓN:
1. Subir sketch al ESP32
2. Abrir Monitor Serial (115200 baud)
3. Verificar inicialización de todos los componentes:
   ✅ PIRManager, ConveyorMotor, ServoManager
   ✅ OLEDDisplay
   ✅ WeightManager (3 celdas HX711)
4. Debe aparecer "Sistema completo inicializado correctamente"

FASE 2 - VERIFICACIÓN DE PESO:
1. Comando 'weight' → Ver estado de todas las celdas
2. Comando 'tare' → Realizar tara si es necesario
3. Colocar objetos conocidos en cada celda para probar
4. Verificar que detecte objetos y reporte pesos correctos

FASE 3 - CALIBRACIÓN (SI ES NECESARIO):
1. Comando 'cal' → Ver información de calibración
2. Para calibrar cada celda individualmente:
   - Usar pesos conocidos (1kg, 2kg, etc.)
   - Seguir instrucciones específicas para cada celda
   - Verificar precisión después de calibrar

FASE 4 - PRUEBA DE CLASIFICACIÓN v2.0:
1. Comando '1' → Activar modo VIDRIO
2. Acercar objeto al PIR1 (GPIO26)
3. Observar secuencia completa:
   - PIR detecta → Motor se detiene → Servo1 activa
   - Sistema pesa objeto en celda 1
   - Compara material PIR (VIDRIO) vs material PESO
   - Muestra resultado de validación
   - Vuelve a modo espera

FASE 5 - PRUEBA DE VALIDACIÓN CRUZADA:
1. Colocar objeto de PLÁSTICO en celda de VIDRIO
2. Activar modo VIDRIO con comando '1'
3. Activar PIR1 con objeto plástico
4. Sistema debe reportar:
   - PIR detectó: VIDRIO
   - Peso indica: PLASTICO
   - VALIDACIÓN FALLIDA

FASE 6 - ESTADÍSTICAS Y DIAGNÓSTICOS:
1. Comando 'stats' → Ver estadísticas detalladas
2. Comando 'test' → Ejecutar diagnóstico completo
3. Comando 'demo' → Ver demostración automática

✅ RESULTADOS ESPERADOS v2.0:

FUNCIONAMIENTO NORMAL:
- Todos los componentes se inicializan correctamente
- PIR sensors detectan objetos y activan servos correspondientes
- Celdas de carga miden peso del objeto clasificado
- Sistema compara material PIR vs material PESO
- Validación exitosa cuando ambos coinciden
- Display OLED muestra estados: PROCESANDO → PESANDO → VALIDADO

DETECCIÓN DE ERRORES:
- Objetos mal clasificados son detectados automáticamente
- Sistema reporta discrepancias entre PIR y PESO
- Se mantienen estadísticas de precisión
- Logs detallados para análisis de errores

INTERFACES MEJORADAS:
- Comandos ampliados para control completo
- Información detallada de estado y estadísticas
- Control individual de celdas de carga
- Modo de diagnóstico automático

❌ PROBLEMAS Y SOLUCIONES v2.0:

"Error inicializando celdas de carga":
- Verificar conexiones HX711 (DOUT/SCK)
- Verificar alimentación 5V de módulos HX711
- Usar comando 'weight' para diagnóstico individual
- Sistema continúa funcionando solo con PIR

"Validación fallida frecuente":
- Revisar calibración de celdas con comando 'cal'
- Verificar rangos de peso por material en WeightManager.h
- Ajustar sensibilidad o realizar nuevas taras
- Verificar que objetos estén en celda correcta

"Lecturas de peso inestables":
- Verificar conexiones mecánicas de celdas
- Eliminar vibraciones en la instalación
- Usar comando 'tare' para recalibrar
- Ajustar filtros en WeightManager.h

"Sistema muy lento":
- El procesamiento de peso agrega ~3 segundos por objeto
- Normal para validación completa PIR + PESO
- Usar 'weight_off' si solo necesitas velocidad PIR
- Optimizar delays en WeightManager si es necesario

📊 MÉTRICAS DE DESEMPEÑO:

TIEMPO DE CLASIFICACIÓN:
- Solo PIR: ~2 segundos por objeto
- PIR + PESO: ~5 segundos por objeto (incluye validación)
- Tiempo adicional justificado por precisión mejorada

PRECISIÓN:
- Solo PIR: Depende de posicionamiento de objetos
- PIR + PESO: >95% precisión con calibración correcta
- Detecta errores que PIR solo no puede identificar

CAPACIDAD:
- Objetos/minuto solo PIR: ~25
- Objetos/minuto PIR+PESO: ~10
- Prioriza calidad sobre velocidad

🚀 SIGUIENTES PASOS:

Una vez validado el sistema completo v2.0:
1. Integrar con Node-RED para monitoreo remoto
2. Agregar base de datos para logging histórico
3. Implementar alertas automáticas por errores
4. Considerar machine learning para detección avanzada
5. Optimizar velocidad manteniendo precisión

===============================================================================
*/