/*
  TEST SIMPLE CLASSIFIER CON LOGGING DE PESO
  ==========================================
  
  Este sketch prueba el sistema SimpleClassifier v1.5 que incluye:
  - Clasificación normal por sensores PIR
  - Control de motor y servos  
  - Display OLED
  - LOGGING DE PESO para base de datos futura
  
  Diferencia con versiones anteriores:
  - El peso NO afecta la clasificación
  - El peso se lee solo para logging/estadísticas
  - Flujo principal sigue siendo PIR → Servo
  - Datos de peso quedan comentados para implementación futura
  
  Hardware necesario:
  - ESP32
  - 3 sensores PIR (GPIO26, 27, 14)
  - Motor con transistor 2N222 (GPIO25)
  - PCA9685 para 3 servos
  - Display OLED SSD1306 (I2C)
  - 3 módulos HX711 con celdas de carga (OPCIONAL)
  
  Flujo de funcionamiento:
  1. Usuario activa modo (1, 2 o 3)
  2. PIR detecta objeto → activa servo correspondiente
  3. Sistema lee peso del objeto (si está habilitado)
  4. Peso se guarda para logging futuro
  5. Clasificación se completa normalmente
*/

// Incluir librerías del proyecto
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/SimpleClassifier.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/SimpleClassifier.cpp"

// Incluir todas las dependencias
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/PIRManager.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/PIRManager.cpp"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ConveyorMotor.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ConveyorMotor.cpp"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/ServoManager.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro\microcontrolador\UpCyclePRO_microcontrolador\ServoManager.cpp"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/OLEDDisplay.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/OLEDDisplay.cpp"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/WeightManager.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/WeightManager.cpp"

// Incluir librería HX711 (opcional)
#include "HX711.h"

// ===============================================================================
// OBJETOS GLOBALES
// ===============================================================================

SimpleClassifier classifier;

// Variables para estadísticas de logging
unsigned long totalClassifications = 0;
unsigned long weightsLogged = 0;

// ===============================================================================
// SETUP
// ===============================================================================

void setup() {
  Serial.begin(115200);
  
  Serial.println();
  Serial.println("🔄 TEST SIMPLE CLASSIFIER v1.5 - UpCycle Pro");
  Serial.println("==============================================");
  Serial.println("✅ Clasificación por PIR (principal)");
  Serial.println("📊 + Logging de peso (futuro/opcional)");
  Serial.println();
  
  // Inicializar sistema
  if (!classifier.begin()) {
    Serial.println("❌ Error inicializando sistema");
    Serial.println("🔄 Verificar conexiones y reiniciar");
    while(1) delay(1000);
  }
  
  Serial.println("✅ Sistema listo para clasificación");
  Serial.println();
  
  printInstructions();
}

// ===============================================================================
// LOOP PRINCIPAL
// ===============================================================================

void loop() {
  // Actualizar clasificador
  classifier.update();
  
  // Procesar comandos seriales
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    command.toLowerCase();
    
    processCommand(command);
  }
  
  delay(50);
}

// ===============================================================================
// PROCESAMIENTO DE COMANDOS
// ===============================================================================

void processCommand(String command) {
  Serial.println("📨 Comando recibido: '" + command + "'");
  
  if (command == "1") {
    Serial.println("🔵 MODO VIDRIO seleccionado");
    classifier.setMode(1);
    totalClassifications++;
    
  } else if (command == "2") {
    Serial.println("🟠 MODO PLASTICO seleccionado");
    classifier.setMode(2);
    totalClassifications++;
    
  } else if (command == "3") {
    Serial.println("⚫ MODO METAL seleccionado");
    classifier.setMode(3);
    totalClassifications++;
    
  } else if (command == "stop") {
    Serial.println("🛑 DETENIENDO SISTEMA");
    classifier.stopSystem();
    
  } else if (command == "status") {
    classifier.printStatus();
    printStats();
    
  } else if (command == "help") {
    printInstructions();
    
  // Comandos de logging de peso
  } else if (command == "weight") {
    classifier.printWeightStatus();
    
  } else if (command == "tare") {
    classifier.tareWeightSensors();
    
  } else if (command == "weight_on") {
    classifier.enableWeightValidation(true);
    Serial.println("📊 Logging de peso HABILITADO para base de datos futura");
    
  } else if (command == "weight_off") {
    classifier.enableWeightValidation(false);
    Serial.println("📡 Solo clasificación PIR (sin logging de peso)");
    
  } else if (command == "stats") {
    printDetailedStats();
    
  } else if (command == "demo") {
    runDemo();
    
  } else {
    Serial.println("❌ Comando no reconocido: '" + command + "'");
    Serial.println("💡 Escribe 'help' para ver comandos disponibles");
  }
  
  Serial.println();
}

// ===============================================================================
// FUNCIONES DE INFORMACIÓN
// ===============================================================================

void printInstructions() {
  Serial.println("📚 COMANDOS DISPONIBLES:");
  Serial.println("========================");
  Serial.println();
  Serial.println("🎯 CLASIFICACIÓN PRINCIPAL:");
  Serial.println("  1, 2, 3       → Activar modo de clasificación PIR");
  Serial.println("  stop          → Detener sistema");
  Serial.println("  status        → Ver estado completo");
  Serial.println("  help          → Mostrar esta ayuda");
  Serial.println();
  Serial.println("📊 LOGGING DE PESO (FUTURO):");
  Serial.println("  weight        → Estado de celdas de carga");
  Serial.println("  tare          → Tara de celdas");
  Serial.println("  weight_on     → Habilitar logging de peso");
  Serial.println("  weight_off    → Solo clasificación PIR");
  Serial.println();
  Serial.println("🧪 DIAGNÓSTICOS:");
  Serial.println("  stats         → Estadísticas detalladas");
  Serial.println("  demo          → Demostración automática");
  Serial.println();
  Serial.println("✨ FUNCIONAMIENTO:");
  Serial.println("  1. El sistema clasifica normalmente por PIR");
  Serial.println("  2. Los pesos se leen solo para logging");
  Serial.println("  3. No hay validación cruzada (flujo simple)");
  Serial.println("  4. Datos quedan listos para base de datos futura");
  Serial.println();
}

void printStats() {
  Serial.println();
  Serial.println("📊 ESTADÍSTICAS BÁSICAS:");
  Serial.println("========================");
  Serial.println("Clasificaciones totales: " + String(totalClassifications));
  Serial.println("Pesos registrados: " + String(weightsLogged));
  Serial.println("Sistema de peso: " + String(classifier.isWeightValidationEnabled() ? "HABILITADO" : "DESHABILITADO"));
  
  if (classifier.isWeightValidationEnabled()) {
    Serial.println("Último peso: " + String(classifier.getDetectedWeight(), 3) + " kg");
  }
}

void printDetailedStats() {
  Serial.println();
  Serial.println("📈 ESTADÍSTICAS DETALLADAS:");
  Serial.println("============================");
  
  printStats();
  classifier.printStatus();
  
  if (classifier.isWeightValidationEnabled()) {
    classifier.printWeightStatus();
  }
  
  Serial.println();
  Serial.println("🔧 CONFIGURACIÓN:");
  Serial.println("  Flujo principal: PIR → Servo");
  Serial.println("  Peso: Solo para logging");
  Serial.println("  Validación cruzada: DESHABILITADA");
  Serial.println("  Base de datos: PREPARADA (TODO)");
}

void runDemo() {
  Serial.println("🎬 DEMOSTRACIÓN DEL SISTEMA:");
  Serial.println("============================");
  Serial.println("📱 Muestra clasificación PIR + logging opcional de peso");
  Serial.println();
  
  // Demo 1: VIDRIO
  Serial.println("🔵 DEMO VIDRIO (5 segundos)");
  classifier.setMode(1);
  delay(5000);
  classifier.stopSystem();
  delay(1000);
  
  // Demo 2: PLASTICO
  Serial.println("🟠 DEMO PLASTICO (5 segundos)");
  classifier.setMode(2);
  delay(5000);
  classifier.stopSystem();
  delay(1000);
  
  // Demo 3: METAL
  Serial.println("⚫ DEMO METAL (5 segundos)");
  classifier.setMode(3);
  delay(5000);
  classifier.stopSystem();
  
  Serial.println("✅ DEMOSTRACIÓN COMPLETADA");
  Serial.println("🎯 Sistema listo para uso normal");
  Serial.println();
}

/*
===============================================================================
                                MANUAL DE USO
===============================================================================

🎯 OBJETIVO:
Probar el sistema UpCycle Pro v1.5 con flujo de clasificación principal por PIR
y logging opcional de peso para base de datos futura.

💡 CONCEPTO:
- CLASIFICACIÓN: 100% basada en sensores PIR (como versión original)
- PESO: Solo se lee para estadísticas y logging futuro
- NO hay validación cruzada que afecte la clasificación
- Datos de peso quedan preparados para envío a base de datos

🔧 HARDWARE:
- ESP32
- 3 sensores PIR (GPIO26, 27, 14) - PRINCIPAL
- Motor + 2N222 (GPIO25)
- 3 servos + PCA9685
- Display OLED SSD1306 (I2C)
- 3 módulos HX711 + celdas de carga - OPCIONAL

📋 PROCEDIMIENTO DE PRUEBA:

FASE 1 - CLASIFICACIÓN BÁSICA:
1. Subir sketch al ESP32
2. Abrir Monitor Serial (115200 baud)
3. Probar comandos 1, 2, 3 para activar modos
4. Verificar que PIR detecte y active servos correspondientes
5. Sistema funciona normalmente sin peso

FASE 2 - LOGGING DE PESO (OPCIONAL):
1. Conectar módulos HX711 si están disponibles
2. Comando 'weight_on' para habilitar logging
3. Comando 'weight' para ver estado de celdas
4. Comando 'tare' si es necesario calibrar
5. Observar que pesos se logueen sin afectar clasificación

FASE 3 - VERIFICACIÓN DE FUNCIONAMIENTO:
1. Activar modo con comando '1'
2. PIR1 detecta objeto → Servo1 se activa
3. Sistema lee peso (si está habilitado) 
4. Muestra "DATOS PARA LOGGING FUTURO" en serial
5. Clasificación se completa normalmente
6. NO hay validación cruzada

FASE 4 - ESTADÍSTICAS:
1. Comando 'stats' para ver estadísticas detalladas
2. Comando 'demo' para demostración automática
3. Verificar que contador de clasificaciones funcione

✅ RESULTADOS ESPERADOS:

FUNCIONAMIENTO NORMAL:
- Clasificación rápida y eficiente por PIR
- Servos se activan según material detectado
- Display OLED muestra estados correctos
- Sin delays adicionales por validación

LOGGING DE PESO:
- Pesos se leen y muestran en serial
- Datos quedan preparados para base de datos
- No interfiere con flujo principal de clasificación
- Comentarios TODO indican dónde implementar envío

FLEXIBILIDAD:
- Sistema funciona sin celdas de carga
- Peso se puede habilitar/deshabilitar dinámicamente
- Mantiene compatibilidad con versiones anteriores

❌ SOLUCIÓN DE PROBLEMAS:

"Error inicializando celdas de carga":
- Normal si no están conectadas
- Sistema continúa funcionando solo con PIR
- Mensaje cambia a "sin logging de peso"

"Logging no funciona":
- Verificar que weight_on esté ejecutado
- Comando 'weight' para diagnosticar celdas
- Usar test_hx711_weight_sensors.ino por separado

"Sistema muy lento":
- No debería pasar con esta versión
- Peso no añade delays significativos
- Solo lectura rápida para logging

📊 VENTAJAS DE ESTA VERSIÓN:

1. **Simplicidad**: Flujo principal sin complicaciones
2. **Velocidad**: No hay delays por validación
3. **Flexibilidad**: Peso opcional y desactivable
4. **Futuro**: Datos listos para base de datos
5. **Compatibilidad**: Funciona sin hardware de peso

🚀 PRÓXIMOS PASOS:

1. Implementar funciones sendToDatabase() y logClassificationEvent()
2. Conectar a sistema MQTT para envío de datos
3. Crear dashboard para visualizar estadísticas de peso
4. Opcional: Añadir análisis de tendencias por tipo de material

Esta versión es ideal para producción donde se necesita clasificación
rápida y confiable con opción de logging para análisis posterior.

===============================================================================
*/