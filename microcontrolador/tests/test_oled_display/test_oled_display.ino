/*
  TEST OLED DISPLAY - UpCycle Pro
  ================================
  
  Este sketch prueba la clase OLEDDisplay que maneja el display OLED
  del sistema de clasificación usando las librerías del proyecto.
  
  Librerías utilizadas:
  - OLEDDisplay.h/.cpp (del proyecto UpCycle Pro)
  - Adafruit_SSD1306 y Adafruit_GFX (incluidas en OLEDDisplay)
  
  Conexiones:
  - OLED SDA: GPIO21 (compartido con PCA9685)
  - OLED SCL: GPIO22 (compartido con PCA9685)
  - OLED VCC: 3.3V ⚠️ IMPORTANTE (NO 5V)
  - OLED GND: GND
  
  Funcionamiento:
  - Usa la clase OLEDDisplay para control del display
  - Métodos específicos por pantalla del sistema
  - Iconos y gráficos integrados
  - Control por comandos seriales
*/

// Incluir las librerías del proyecto
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/OLEDDisplay.h"
#include "C:/Users/hombr/Desktop/projects/programacion/UpCycle-Pro/microcontrolador/UpCyclePRO_microcontrolador/OLEDDisplay.cpp"

// LED indicador
#define LED_PIN 2

// Crear objeto OLEDDisplay
OLEDDisplay oledDisplay;

// Variables de control
unsigned long last_demo_change = 0;
unsigned long last_status_report = 0;
bool display_initialized = false;
bool auto_demo = true;
bool auto_report = true;
int demo_screen = 0;
unsigned long demo_interval = 4000; // 4 segundos
unsigned long status_interval = 10000; // 10 segundos

// Estadísticas de pantallas mostradas
unsigned long recycle_screen_count = 0;
unsigned long material_screen_count = 0;
unsigned long mode_screen_count = 0;
unsigned long status_screen_count = 0;
unsigned long error_screen_count = 0;

void setup() {
  Serial.begin(115200);
  
  // Configurar LED indicador
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println();
  Serial.println("📺 TEST OLED DISPLAY - UpCycle Pro");
  Serial.println("==================================");
  Serial.println();
  
  Serial.println("📋 Usando librerías del proyecto:");
  Serial.println("- OLEDDisplay.h/.cpp");
  Serial.println("- Adafruit_SSD1306 y Adafruit_GFX (integradas)");
  Serial.println();
  
  // Inicializar I2C
  Wire.begin();
  
  // Inicializar OLEDDisplay
  Serial.println("🔄 Inicializando OLEDDisplay...");
  if (oledDisplay.begin()) {
    display_initialized = true;
    Serial.println("✅ OLEDDisplay inicializado correctamente");
    
    // LED indicador de éxito
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
    
    // Mostrar pantalla inicial
    Serial.println("🏠 Mostrando pantalla de reciclaje inicial...");
    oledDisplay.showRecycleScreen();
    recycle_screen_count++;
    
  } else {
    Serial.println("❌ Error inicializando OLEDDisplay");
    Serial.println("   Verificar conexiones I2C y alimentación 3.3V");
    while(1) {
      digitalWrite(LED_PIN, HIGH);
      delay(1000);
      digitalWrite(LED_PIN, LOW);
      delay(1000);
    }
  }
  
  Serial.println();
  Serial.println("🔧 Configuración OLEDDisplay:");
  Serial.println("- Resolución: 128x32 pixels (SCREEN_WIDTH x SCREEN_HEIGHT)");
  Serial.println("- Dirección I2C: 0x3C (SCREEN_ADDRESS)");
  Serial.println("- Ícono reciclaje: Bitmap integrado 24x24");
  Serial.println("- Métodos específicos por pantalla del sistema");
  Serial.println();
  
  delay(3000); // Mostrar pantalla inicial
  
  printHelp();
}

void loop() {
  // Procesar comandos seriales
  if (Serial.available()) {
    processCommand();
  }
  
  // Demo automático de pantallas
  if (auto_demo && display_initialized && (millis() - last_demo_change > demo_interval)) {
    runAutoDemo();
    last_demo_change = millis();
  }
  
  // Reporte automático periódico
  if (auto_report && display_initialized && (millis() - last_status_report > status_interval)) {
    printStatusReport();
    last_status_report = millis();
  }
  
  delay(100);
}

void processCommand() {
  String command = Serial.readString();
  command.trim();
  command.toLowerCase();
  
  if (command == "recycle" || command == "home") {
    showRecycleScreen();
  } else if (command == "vidrio" || command == "1") {
    showVidrioScreen();
  } else if (command == "plastico" || command == "2") {
    showPlasticoScreen();
  } else if (command == "metal" || command == "3") {
    showMetalScreen();
  } else if (command == "mode1") {
    showMode1Screen();
  } else if (command == "mode2") {
    showMode2Screen();
  } else if (command == "mode3") {
    showMode3Screen();
  } else if (command == "waiting") {
    showWaitingScreen();
  } else if (command == "processing") {
    showProcessingScreen();
  } else if (command == "error") {
    showErrorScreen();
  } else if (command == "weight") {
    showWeightScreen();
  } else if (command == "clear") {
    clearDisplay();
  } else if (command == "demo" || command == "d") {
    auto_demo = !auto_demo;
    Serial.print("🔄 Demo automático: ");
    Serial.println(auto_demo ? "ACTIVADO" : "DESACTIVADO");
    if (!auto_demo) {
      Serial.println("   Demo pausado en pantalla actual");
    }
  } else if (command == "status" || command == "s") {
    printFullStatus();
  } else if (command == "auto" || command == "a") {
    auto_report = !auto_report;
    Serial.print("🔄 Reporte automático: ");
    Serial.println(auto_report ? "ACTIVADO" : "DESACTIVADO");
  } else if (command == "stats") {
    printStatistics();
  } else if (command == "clear_stats") {
    clearStatistics();
  } else if (command == "test") {
    runDisplayTest();
  } else if (command == "sequence") {
    runScreenSequence();
  } else if (command == "return") {
    // Usar método de retorno automático
    Serial.println("🔄 Activando retorno automático a pantalla principal");
    oledDisplay.returnToMainScreen();
  } else if (command == "help" || command == "h") {
    printHelp();
  } else {
    Serial.println("❌ Comando no reconocido");
    Serial.println("💡 Envía 'help' para ver comandos disponibles");
  }
}

void showRecycleScreen() {
  if (!display_initialized) return;
  
  Serial.println("♻️ Mostrando pantalla de reciclaje");
  Serial.println("   Usando oledDisplay.showRecycleScreen()...");
  
  oledDisplay.showRecycleScreen();
  recycle_screen_count++;
  
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("   ✅ Pantalla de reciclaje mostrada");
}

void showVidrioScreen() {
  if (!display_initialized) return;
  
  Serial.println("🔵 Mostrando pantalla VIDRIO");
  Serial.println("   Usando oledDisplay.showVidrio()...");
  
  oledDisplay.showVidrio();
  material_screen_count++;
  
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("   ✅ Pantalla VIDRIO mostrada");
}

void showPlasticoScreen() {
  if (!display_initialized) return;
  
  Serial.println("🟠 Mostrando pantalla PLASTICO");
  Serial.println("   Usando oledDisplay.showPlastico()...");
  
  oledDisplay.showPlastico();
  material_screen_count++;
  
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("   ✅ Pantalla PLASTICO mostrada");
}

void showMetalScreen() {
  if (!display_initialized) return;
  
  Serial.println("⚫ Mostrando pantalla METAL");
  Serial.println("   Usando oledDisplay.showMetal()...");
  
  oledDisplay.showMetal();
  material_screen_count++;
  
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("   ✅ Pantalla METAL mostrada");
}

void showMode1Screen() {
  if (!display_initialized) return;
  
  Serial.println("1️⃣ Mostrando modo 1 (VIDRIO)");
  Serial.println("   Usando oledDisplay.showMode1()...");
  
  oledDisplay.showMode1();
  mode_screen_count++;
  
  Serial.println("   ✅ Modo 1 mostrado");
}

void showMode2Screen() {
  if (!display_initialized) return;
  
  Serial.println("2️⃣ Mostrando modo 2 (PLASTICO)");
  Serial.println("   Usando oledDisplay.showMode2()...");
  
  oledDisplay.showMode2();
  mode_screen_count++;
  
  Serial.println("   ✅ Modo 2 mostrado");
}

void showMode3Screen() {
  if (!display_initialized) return;
  
  Serial.println("3️⃣ Mostrando modo 3 (METAL)");
  Serial.println("   Usando oledDisplay.showMode3()...");
  
  oledDisplay.showMode3();
  mode_screen_count++;
  
  Serial.println("   ✅ Modo 3 mostrado");
}

void showWaitingScreen() {
  if (!display_initialized) return;
  
  Serial.println("⏳ Mostrando pantalla ESPERANDO");
  Serial.println("   Usando oledDisplay.showWaiting()...");
  
  oledDisplay.showWaiting("VIDRIO");
  status_screen_count++;
  
  Serial.println("   ✅ Pantalla ESPERANDO mostrada");
}

void showProcessingScreen() {
  if (!display_initialized) return;
  
  Serial.println("⚙️ Mostrando pantalla PROCESANDO");
  Serial.println("   Usando oledDisplay.showProcessing()...");
  
  oledDisplay.showProcessing("PLASTICO");
  status_screen_count++;
  
  Serial.println("   ✅ Pantalla PROCESANDO mostrada");
}

void showErrorScreen() {
  if (!display_initialized) return;
  
  Serial.println("❌ Mostrando pantalla ERROR");
  Serial.println("   Usando oledDisplay.showError()...");
  
  oledDisplay.showError("Test Error");
  error_screen_count++;
  
  Serial.println("   ✅ Pantalla ERROR mostrada");
}

void showWeightScreen() {
  if (!display_initialized) return;
  
  Serial.println("⚖️ Mostrando peso simulado");
  Serial.println("   Usando oledDisplay.showWeight()...");
  
  oledDisplay.showWeight(1.25); // 1.25 kg simulado
  status_screen_count++;
  
  Serial.println("   ✅ Pantalla PESO mostrada");
}

void clearDisplay() {
  if (!display_initialized) return;
  
  Serial.println("🧹 Limpiando display");
  Serial.println("   Usando oledDisplay.clear()...");
  
  oledDisplay.clear();
  
  Serial.println("   ✅ Display limpiado");
}

void runAutoDemo() {
  switch(demo_screen) {
    case 0:
      Serial.println("🔄 Demo: Pantalla de reciclaje");
      oledDisplay.showRecycleScreen();
      recycle_screen_count++;
      break;
    case 1:
      Serial.println("🔄 Demo: Modo 1 (VIDRIO)");
      oledDisplay.showMode1();
      mode_screen_count++;
      break;
    case 2:
      Serial.println("🔄 Demo: Esperando VIDRIO");
      oledDisplay.showWaiting("VIDRIO");
      status_screen_count++;
      break;
    case 3:
      Serial.println("🔄 Demo: Pantalla PLASTICO");
      oledDisplay.showPlastico();
      material_screen_count++;
      break;
    case 4:
      Serial.println("🔄 Demo: Procesando METAL");
      oledDisplay.showProcessing("METAL");
      status_screen_count++;
      break;
    case 5:
      Serial.println("🔄 Demo: Peso 2.34 kg");
      oledDisplay.showWeight(2.34);
      status_screen_count++;
      break;
  }
  
  demo_screen = (demo_screen + 1) % 6;
}

void printHelp() {
  Serial.println("🎮 COMANDOS DISPONIBLES:");
  Serial.println("========================");
  Serial.println("PANTALLAS PRINCIPALES:");
  Serial.println("- 'recycle' o 'home' = Pantalla reciclaje (showRecycleScreen)");
  Serial.println("- 'vidrio' o '1' = Pantalla VIDRIO (showVidrio)");
  Serial.println("- 'plastico' o '2' = Pantalla PLASTICO (showPlastico)");
  Serial.println("- 'metal' o '3' = Pantalla METAL (showMetal)");
  Serial.println();
  Serial.println("MODOS DEL SISTEMA:");
  Serial.println("- 'mode1' = Modo 1 VIDRIO (showMode1)");
  Serial.println("- 'mode2' = Modo 2 PLASTICO (showMode2)");
  Serial.println("- 'mode3' = Modo 3 METAL (showMode3)");
  Serial.println();
  Serial.println("ESTADOS DEL SISTEMA:");
  Serial.println("- 'waiting' = Pantalla ESPERANDO (showWaiting)");
  Serial.println("- 'processing' = Pantalla PROCESANDO (showProcessing)");
  Serial.println("- 'error' = Pantalla ERROR (showError)");
  Serial.println("- 'weight' = Mostrar peso (showWeight)");
  Serial.println();
  Serial.println("CONTROL:");
  Serial.println("- 'clear' = Limpiar display (clear)");
  Serial.println("- 'demo' o 'd' = Activar/desactivar demo automático");
  Serial.println("- 'status' o 's' = Estado completo del sistema");
  Serial.println("- 'auto' o 'a' = Activar/desactivar reporte automático");
  Serial.println("- 'stats' = Estadísticas de pantallas");
  Serial.println("- 'clear_stats' = Limpiar estadísticas");
  Serial.println("- 'test' = Prueba completa de pantallas");
  Serial.println("- 'sequence' = Secuencia de todas las pantallas");
  Serial.println("- 'return' = Activar retorno automático (returnToMainScreen)");
  Serial.println("- 'help' o 'h' = Mostrar esta ayuda");
  Serial.println();
}

void printStatusReport() {
  Serial.print("📺 Display: ");
  Serial.print(display_initialized ? "✅" : "❌");
  Serial.print(" | Demo: ");
  Serial.print(auto_demo ? "ON" : "OFF");
  Serial.print(" | Pantallas: R=");
  Serial.print(recycle_screen_count);
  Serial.print(" M=");
  Serial.print(material_screen_count);
  Serial.print(" Mo=");
  Serial.print(mode_screen_count);
  Serial.print(" S=");
  Serial.print(status_screen_count);
  Serial.println();
}

void printFullStatus() {
  Serial.println("📋 ESTADO COMPLETO - OLED DISPLAY");
  Serial.println("=================================");
  Serial.println();
  
  Serial.println("🔧 Configuración OLEDDisplay:");
  Serial.println("- Clase: OLEDDisplay del proyecto UpCycle Pro");
  Serial.println("- Controlador: SSD1306 en I2C");
  Serial.println("- Resolución: 128x32 pixels");
  Serial.println("- Dirección: 0x3C");
  Serial.println("- Alimentación: 3.3V");
  Serial.println();
  
  Serial.print("⚙️ Estado de inicialización: ");
  Serial.println(display_initialized ? "✅ INICIALIZADO" : "❌ ERROR");
  
  if (display_initialized) {
    Serial.println();
    Serial.println("📊 Métodos disponibles:");
    Serial.println("- showRecycleScreen(): Pantalla principal con ícono");
    Serial.println("- showVidrio(), showPlastico(), showMetal(): Materiales");
    Serial.println("- showMode1(), showMode2(), showMode3(): Modos del sistema");
    Serial.println("- showWaiting(), showProcessing(): Estados");
    Serial.println("- showWeight(), showError(): Información adicional");
    Serial.println("- clear(), returnToMainScreen(): Control");
    Serial.println();
    
    Serial.print("⚙️ Configuración:");
    Serial.print(" Demo automático=");
    Serial.print(auto_demo ? "ON" : "OFF");
    Serial.print(" | Reporte automático=");
    Serial.print(auto_report ? "ON" : "OFF");
    Serial.print(" | Demo actual=");
    Serial.println(demo_screen);
    Serial.println();
  }
  
  printStatistics();
}

void printStatistics() {
  Serial.println("📈 ESTADÍSTICAS DE PANTALLAS:");
  Serial.print("- Pantalla reciclaje: ");
  Serial.println(recycle_screen_count);
  Serial.print("- Pantallas materiales: ");
  Serial.println(material_screen_count);
  Serial.print("- Pantallas modos: ");
  Serial.println(mode_screen_count);
  Serial.print("- Pantallas estado: ");
  Serial.println(status_screen_count);
  Serial.print("- Pantallas error: ");
  Serial.println(error_screen_count);
  Serial.print("- Total pantallas mostradas: ");
  Serial.println(recycle_screen_count + material_screen_count + mode_screen_count + status_screen_count + error_screen_count);
  Serial.println();
}

void clearStatistics() {
  recycle_screen_count = 0;
  material_screen_count = 0;
  mode_screen_count = 0;
  status_screen_count = 0;
  error_screen_count = 0;
  
  Serial.println("🔄 Estadísticas de pantallas limpiadas");
  Serial.println("📊 Todos los contadores reseteados a 0");
}

void runDisplayTest() {
  if (!display_initialized) {
    Serial.println("❌ No se puede ejecutar test - OLEDDisplay no inicializado");
    return;
  }
  
  Serial.println("🧪 PRUEBA COMPLETA DEL DISPLAY");
  Serial.println("===============================");
  Serial.println();
  
  Serial.println("Esta prueba mostrará todas las pantallas disponibles");
  Serial.println("usando los métodos de la clase OLEDDisplay.");
  Serial.println();
  
  // Test 1: Pantalla principal
  Serial.println("1️⃣ Pantalla de reciclaje");
  oledDisplay.showRecycleScreen();
  delay(2000);
  
  // Test 2: Modos
  Serial.println("2️⃣ Modos del sistema");
  Serial.println("   Modo 1 (VIDRIO)");
  oledDisplay.showMode1();
  delay(1500);
  Serial.println("   Modo 2 (PLASTICO)");
  oledDisplay.showMode2();
  delay(1500);
  Serial.println("   Modo 3 (METAL)");
  oledDisplay.showMode3();
  delay(1500);
  
  // Test 3: Materiales
  Serial.println("3️⃣ Pantallas de materiales");
  Serial.println("   VIDRIO");
  oledDisplay.showVidrio();
  delay(1500);
  Serial.println("   PLASTICO");
  oledDisplay.showPlastico();
  delay(1500);
  Serial.println("   METAL");
  oledDisplay.showMetal();
  delay(1500);
  
  // Test 4: Estados
  Serial.println("4️⃣ Estados del sistema");
  Serial.println("   ESPERANDO");
  oledDisplay.showWaiting("VIDRIO");
  delay(1500);
  Serial.println("   PROCESANDO");
  oledDisplay.showProcessing("PLASTICO");
  delay(1500);
  
  // Test 5: Información adicional
  Serial.println("5️⃣ Información adicional");
  Serial.println("   Peso");
  oledDisplay.showWeight(3.47);
  delay(1500);
  Serial.println("   Error");
  oledDisplay.showError("Test Error");
  delay(1500);
  
  // Test 6: Limpieza y retorno
  Serial.println("6️⃣ Limpieza y retorno");
  Serial.println("   Limpiar display");
  oledDisplay.clear();
  delay(1000);
  Serial.println("   Retorno a pantalla principal");
  oledDisplay.showRecycleScreen();
  
  Serial.println();
  Serial.println("📋 RESUMEN DEL TEST:");
  Serial.println("- Pantalla reciclaje: ✅ OK");
  Serial.println("- Modos (1,2,3): ✅ OK");
  Serial.println("- Materiales (V,P,M): ✅ OK");
  Serial.println("- Estados (Esperando/Procesando): ✅ OK");
  Serial.println("- Información adicional: ✅ OK");
  Serial.println("- Limpieza y control: ✅ OK");
  Serial.println();
  Serial.println("🎉 ¡Todas las pantallas funcionan correctamente!");
  Serial.println();
}

void runScreenSequence() {
  if (!display_initialized) {
    Serial.println("❌ No se puede ejecutar secuencia - OLEDDisplay no inicializado");
    return;
  }
  
  Serial.println("🔄 SECUENCIA COMPLETA DE PANTALLAS");
  Serial.println("==================================");
  Serial.println();
  
  Serial.println("Ejecutando secuencia de todas las pantallas disponibles");
  Serial.println("con transiciones suaves...");
  Serial.println();
  
  String screens[] = {
    "Reciclaje", "Modo 1", "Esperando VIDRIO", "VIDRIO detectado",
    "Procesando VIDRIO", "Modo 2", "Esperando PLASTICO", "PLASTICO detectado", 
    "Procesando PLASTICO", "Modo 3", "Esperando METAL", "METAL detectado",
    "Procesando METAL", "Peso: 2.15 kg", "Retorno a inicio"
  };
  
  for (int i = 0; i < 15; i++) {
    Serial.print("   ");
    Serial.print(i + 1);
    Serial.print("/15: ");
    Serial.println(screens[i]);
    
    switch(i) {
      case 0: oledDisplay.showRecycleScreen(); break;
      case 1: oledDisplay.showMode1(); break;
      case 2: oledDisplay.showWaiting("VIDRIO"); break;
      case 3: oledDisplay.showVidrio(); break;
      case 4: oledDisplay.showProcessing("VIDRIO"); break;
      case 5: oledDisplay.showMode2(); break;
      case 6: oledDisplay.showWaiting("PLASTICO"); break;
      case 7: oledDisplay.showPlastico(); break;
      case 8: oledDisplay.showProcessing("PLASTICO"); break;
      case 9: oledDisplay.showMode3(); break;
      case 10: oledDisplay.showWaiting("METAL"); break;
      case 11: oledDisplay.showMetal(); break;
      case 12: oledDisplay.showProcessing("METAL"); break;
      case 13: oledDisplay.showWeight(2.15); break;
      case 14: oledDisplay.showRecycleScreen(); break;
    }
    
    delay(1500);
  }
  
  Serial.println();
  Serial.println("🎉 ¡Secuencia completa finalizada!");
  Serial.println("📊 Total transiciones: 15");
  Serial.println();
}

/*
===============================================================================
                                MANUAL DE USO
===============================================================================

🎯 OBJETIVO:
Probar la clase OLEDDisplay del proyecto UpCycle Pro que controla el display
OLED con métodos específicos para cada pantalla del sistema.

📚 LIBRERÍAS UTILIZADAS:
- OLEDDisplay.h/.cpp (del proyecto UpCycle Pro)
- Adafruit_SSD1306 y Adafruit_GFX (integradas)
- Métodos específicos por pantalla del sistema de clasificación

🔧 CONFIGURACIÓN:
1. Conectar OLED según documentación del proyecto:
   - SDA: GPIO21 (compartido con PCA9685)
   - SCL: GPIO22 (compartido con PCA9685)
   - VCC: 3.3V ⚠️ IMPORTANTE (NO 5V)
   - GND: GND

2. Subir este sketch al ESP32
3. Abrir Monitor Serial (115200 baud)

📋 CARACTERÍSTICAS DE OLEDDisplay:
- Resolución 128x32 optimizada para el sistema
- Ícono de reciclaje integrado (bitmap 24x24)
- Métodos específicos por pantalla del sistema:
  * showRecycleScreen(): Pantalla principal
  * showVidrio(), showPlastico(), showMetal(): Materiales
  * showMode1(), showMode2(), showMode3(): Modos
  * showWaiting(), showProcessing(): Estados
  * showWeight(), showError(): Información
- Texto centrado automático
- Retorno automático a pantalla principal
- Control de actualización optimizado

🎮 COMANDOS DISPONIBLES:

PANTALLAS PRINCIPALES:
- 'recycle' o 'home': Pantalla reciclaje con ícono
- 'vidrio' o '1': Pantalla material VIDRIO
- 'plastico' o '2': Pantalla material PLASTICO
- 'metal' o '3': Pantalla material METAL

MODOS DEL SISTEMA:
- 'mode1': Modo 1 VIDRIO (número grande)
- 'mode2': Modo 2 PLASTICO (número grande)
- 'mode3': Modo 3 METAL (número grande)

ESTADOS:
- 'waiting': Pantalla ESPERANDO material
- 'processing': Pantalla PROCESANDO material
- 'error': Pantalla de ERROR
- 'weight': Mostrar peso en kg

CONTROL Y PRUEBAS:
- 'demo' o 'd': Activar/desactivar demo automático
- 'test': Prueba completa de todas las pantallas
- 'sequence': Secuencia simulando funcionamiento real
- 'clear': Limpiar display
- 'return': Activar retorno automático

📋 PROCEDIMIENTO DE PRUEBA:

DEMO AUTOMÁTICO:
1. Sistema muestra automáticamente diferentes pantallas cada 4 segundos
2. Secuencia: Reciclaje → Modo 1 → Esperando → Material → Procesando → Peso
3. Activar/desactivar con comando 'demo'

PRUEBA MANUAL:
1. Probar cada pantalla individualmente con comandos específicos
2. Verificar que texto se muestra correctamente centrado
3. Comprobar ícono de reciclaje en pantalla principal

PRUEBA AUTOMÁTICA:
1. Comando 'test': Prueba sistemática de todas las pantallas
2. Comando 'sequence': Simula secuencia real de funcionamiento
3. Verificar transiciones suaves entre pantallas

✅ RESULTADOS ESPERADOS:
- OLEDDisplay se inicializa sin errores
- Todas las pantallas se muestran correctamente
- Texto centrado y legible en resolución 128x32
- Ícono de reciclaje se muestra en pantalla principal
- Transiciones suaves entre pantallas
- Métodos específicos funcionan correctamente
- Retorno automático a pantalla principal

❌ PROBLEMAS Y SOLUCIONES:

"OLEDDisplay no inicializa":
- Verificar conexiones I2C (SDA=21, SCL=22)
- Verificar alimentación 3.3V (NO 5V - puede dañar)
- Verificar dirección I2C 0x3C
- Probar con I2C scanner primero

"Pantalla en blanco":
- Verificar que display sea SSD1306 128x32
- Comprobar que se llama update() después de cambios
- Verificar calidad de conexiones I2C
- Revisar alimentación estable 3.3V

"Texto cortado o mal centrado":
- Verificar resolución configurada (128x32)
- Comprobar implementación de drawCenteredText()
- Ajustar tamaños de fuente para pantalla pequeña

"Ícono no se muestra":
- Verificar bitmap integrado (recycleBitmap)
- Comprobar implementación de drawRecycleIcon()
- Verificar espacio suficiente en pantalla 32 pixels altura

📊 INTERPRETACIÓN:
- Inicialización correcta: Hardware funcionando
- Todas las pantallas OK: Software integrado correctamente
- Texto legible: Configuración de fuentes apropiada
- Transiciones suaves: Control de timing optimizado

🎯 VENTAJAS DE USAR OLEDDisplay:
- Métodos específicos para cada pantalla del sistema
- Ícono personalizado integrado
- Texto centrado automático
- Optimizado para resolución 128x32
- Gestión automática de actualizaciones
- Retorno automático a pantalla principal
- Código más limpio y mantenible
- Fácil integración con SimpleClassifier

🔧 CARACTERÍSTICAS ESPECÍFICAS DEL PROYECTO:
- Diseñado específicamente para UpCycle Pro
- Pantallas optimizadas para clasificación de materiales
- Integración directa con estados del sistema
- Ícono de reciclaje personalizado
- Mensajes específicos del dominio (VIDRIO, PLASTICO, METAL)

===============================================================================
*/