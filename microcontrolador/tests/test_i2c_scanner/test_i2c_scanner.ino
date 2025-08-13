/*
  TEST I2C SCANNER - UpCycle Pro
  ===============================
  
  Este sketch escanea el bus I2C para detectar todos los dispositivos
  conectados. Útil para diagnosticar problemas de comunicación y
  verificar direcciones de dispositivos.
  
  Dispositivos esperados en UpCycle Pro:
  - PCA9685 (Controlador de servos): 0x40 (por defecto)
  - OLED SSD1306 (Display): 0x3C o 0x3D
  
  Conexiones I2C:
  - SDA: GPIO21
  - SCL: GPIO22
  - Alimentación: 3.3V para OLED, 5V para PCA9685
  
  Funcionamiento:
  - Escanea direcciones I2C de 0x08 a 0x77
  - Reporta dispositivos encontrados
  - Análisis de velocidad I2C
  - Pruebas de comunicación básica
*/

#include <Wire.h>

// Configuración I2C
#define SDA_PIN 21
#define SCL_PIN 22

// Direcciones conocidas del sistema
#define PCA9685_ADDR 0x40  // Controlador de servos
#define OLED_ADDR1 0x3C    // Display OLED opción 1
#define OLED_ADDR2 0x3D    // Display OLED opción 2

// Variables de control
unsigned long last_scan = 0;
bool auto_scan = true;
unsigned long scan_interval = 10000; // Escaneo cada 10 segundos
int devices_found = 0;

// Estructura para dispositivos conocidos
struct KnownDevice {
  uint8_t address;
  String name;
  String description;
};

KnownDevice known_devices[] = {
  {0x3C, "OLED SSD1306", "Display 128x32/64"},
  {0x3D, "OLED SSD1306", "Display 128x32/64 (addr alt)"},
  {0x40, "PCA9685", "Controlador PWM 16 canales"},
  {0x41, "PCA9685", "Controlador PWM (addr+1)"},
  {0x48, "ADS1115", "ADC 16-bit"},
  {0x49, "ADS1115", "ADC 16-bit (addr+1)"},
  {0x50, "EEPROM", "Memoria EEPROM"},
  {0x51, "EEPROM", "Memoria EEPROM (addr+1)"},
  {0x68, "DS1307/DS3231", "Real Time Clock"},
  {0x76, "BMP280", "Sensor presión/temperatura"},
  {0x77, "BMP280", "Sensor presión/temperatura (addr alt)"}
};

const int num_known_devices = sizeof(known_devices) / sizeof(known_devices[0]);

void setup() {
  Serial.begin(115200);
  
  Serial.println();
  Serial.println("🔍 I2C SCANNER - UpCycle Pro");
  Serial.println("=============================");
  Serial.println();
  
  // Inicializar I2C con pines específicos
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Configurar velocidad I2C (100kHz estándar)
  Wire.setClock(100000);
  
  Serial.println("📋 Configuración I2C:");
  Serial.print("- SDA: GPIO");
  Serial.println(SDA_PIN);
  Serial.print("- SCL: GPIO");
  Serial.println(SCL_PIN);
  Serial.println("- Velocidad: 100kHz");
  Serial.println();
  
  Serial.println("🎯 Dispositivos esperados:");
  Serial.println("- PCA9685 (Servos): 0x40");
  Serial.println("- OLED SSD1306: 0x3C o 0x3D");
  Serial.println();
  
  // Primer escaneo inmediato
  Serial.println("🔄 Ejecutando primer escaneo...");
  Serial.println();
  performScan();
  
  Serial.println();
  Serial.println("🎮 COMANDOS:");
  Serial.println("- 's' = Escanear dispositivos I2C");
  Serial.println("- 'a' = Activar/Desactivar escaneo automático");
  Serial.println("- 't' = Probar comunicación con dispositivos");
  Serial.println("- 'v' = Cambiar velocidad I2C");
  Serial.println("- 'r' = Reset bus I2C");
  Serial.println("- 'h' = Ayuda y diagnóstico");
  Serial.println();
}

void loop() {
  // Procesar comandos seriales
  if (Serial.available()) {
    char command = Serial.read();
    processCommand(command);
  }
  
  // Escaneo automático si está activado
  if (auto_scan && (millis() - last_scan > scan_interval)) {
    Serial.println("🔄 Escaneo automático:");
    performScan();
    last_scan = millis();
  }
  
  delay(100);
}

void processCommand(char cmd) {
  switch(cmd) {
    case 's':
      Serial.println("🔍 Escaneando dispositivos I2C...");
      performScan();
      break;
      
    case 'a':
      auto_scan = !auto_scan;
      Serial.print("🔄 Escaneo automático: ");
      Serial.println(auto_scan ? "ACTIVADO" : "DESACTIVADO");
      if (auto_scan) {
        last_scan = millis();
      }
      break;
      
    case 't':
      Serial.println("🧪 Probando comunicación con dispositivos...");
      testCommunication();
      break;
      
    case 'v':
      changeI2CSpeed();
      break;
      
    case 'r':
      Serial.println("🔄 Reiniciando bus I2C...");
      resetI2C();
      break;
      
    case 'h':
      showDiagnostics();
      break;
      
    default:
      Serial.println("❌ Comando no reconocido");
      Serial.println("💡 Comandos: s, a, t, v, r, h");
      break;
  }
}

void performScan() {
  devices_found = 0;
  
  Serial.println("📊 Escaneando direcciones 0x08 - 0x77...");
  Serial.println();
  
  for (uint8_t address = 8; address < 120; address++) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    
    if (error == 0) {
      devices_found++;
      Serial.print("✅ Dispositivo encontrado en 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.print(" (");
      Serial.print(address);
      Serial.print(")");
      
      // Buscar en dispositivos conocidos
      String device_info = getDeviceInfo(address);
      if (device_info.length() > 0) {
        Serial.print(" - ");
        Serial.print(device_info);
      }
      Serial.println();
      
      // Análisis específico para dispositivos del sistema
      analyzeDevice(address);
      
    } else if (error == 4) {
      Serial.print("❌ Error desconocido en 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
    
    delay(10); // Pequeña pausa entre direcciones
  }
  
  Serial.println();
  Serial.print("📈 Resumen: ");
  Serial.print(devices_found);
  Serial.println(" dispositivo(s) encontrado(s)");
  
  if (devices_found == 0) {
    Serial.println();
    Serial.println("⚠️ NO SE ENCONTRARON DISPOSITIVOS");
    Serial.println("🔧 Verificar:");
    Serial.println("   - Conexiones SDA/SCL (GPIO21/22)");
    Serial.println("   - Alimentación de dispositivos");
    Serial.println("   - Resistencias pull-up (4.7kΩ)");
    Serial.println("   - Cables no muy largos (<30cm)");
  }
  
  Serial.println();
}

String getDeviceInfo(uint8_t address) {
  for (int i = 0; i < num_known_devices; i++) {
    if (known_devices[i].address == address) {
      return known_devices[i].name + " (" + known_devices[i].description + ")";
    }
  }
  return "";
}

void analyzeDevice(uint8_t address) {
  if (address == PCA9685_ADDR) {
    Serial.println("   🎛️ PCA9685 detectado - Controlador de servos OK");
  } else if (address == OLED_ADDR1 || address == OLED_ADDR2) {
    Serial.println("   📺 OLED detectado - Display OK");
  }
}

void testCommunication() {
  if (devices_found == 0) {
    Serial.println("❌ No hay dispositivos para probar");
    return;
  }
  
  Serial.println("🧪 Probando comunicación con cada dispositivo...");
  Serial.println();
  
  for (uint8_t address = 8; address < 120; address++) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    
    if (error == 0) {
      // Dispositivo presente, hacer prueba de comunicación
      Serial.print("🔄 Probando 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.print(": ");
      
      // Intentar leer datos
      Wire.requestFrom(address, (uint8_t)1);
      unsigned long timeout = millis() + 100;
      
      while (Wire.available() == 0 && millis() < timeout) {
        delay(1);
      }
      
      if (Wire.available()) {
        uint8_t data = Wire.read();
        Serial.print("Respuesta OK (0x");
        if (data < 16) Serial.print("0");
        Serial.print(data, HEX);
        Serial.println(")");
      } else {
        Serial.println("Sin respuesta de lectura");
      }
    }
  }
  Serial.println();
}

void changeI2CSpeed() {
  Serial.println("⚡ Velocidades I2C disponibles:");
  Serial.println("1. 100kHz (Estándar)");
  Serial.println("2. 400kHz (Rápido)");
  Serial.println("3. 1MHz (Rápido+)");
  Serial.println();
  Serial.println("Envía 1, 2 o 3 para cambiar velocidad...");
  
  // Esperar entrada del usuario
  while (!Serial.available()) {
    delay(100);
  }
  
  char choice = Serial.read();
  uint32_t new_speed;
  
  switch (choice) {
    case '1':
      new_speed = 100000;
      Serial.println("🐌 Velocidad: 100kHz (Estándar)");
      break;
    case '2':
      new_speed = 400000;
      Serial.println("🏃 Velocidad: 400kHz (Rápido)");
      break;
    case '3':
      new_speed = 1000000;
      Serial.println("🚀 Velocidad: 1MHz (Rápido+)");
      break;
    default:
      Serial.println("❌ Opción inválida");
      return;
  }
  
  Wire.setClock(new_speed);
  Serial.println("✅ Velocidad cambiada. Reescaneando...");
  Serial.println();
  performScan();
}

void resetI2C() {
  Wire.end();
  delay(100);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  Serial.println("✅ Bus I2C reiniciado");
  Serial.println("🔄 Reescaneando dispositivos...");
  Serial.println();
  performScan();
}

void showDiagnostics() {
  Serial.println("🔧 DIAGNÓSTICO DEL SISTEMA I2C");
  Serial.println("===============================");
  Serial.println();
  
  Serial.println("📋 Configuración actual:");
  Serial.print("- SDA: GPIO");
  Serial.println(SDA_PIN);
  Serial.print("- SCL: GPIO");
  Serial.println(SCL_PIN);
  Serial.println("- Velocidad: 100kHz");
  Serial.print("- Dispositivos activos: ");
  Serial.println(devices_found);
  Serial.print("- Escaneo automático: ");
  Serial.println(auto_scan ? "ACTIVO" : "INACTIVO");
  Serial.println();
  
  Serial.println("🎯 Dispositivos esperados para UpCycle Pro:");
  Serial.println("- 0x40: PCA9685 (Controlador de servos)");
  Serial.println("- 0x3C: OLED SSD1306 (Display principal)");
  Serial.println("- 0x3D: OLED SSD1306 (Dirección alternativa)");
  Serial.println();
  
  Serial.println("⚠️ Problemas comunes y soluciones:");
  Serial.println();
  Serial.println("🔴 No se detectan dispositivos:");
  Serial.println("   - Verificar conexiones SDA/SCL");
  Serial.println("   - Verificar alimentación (3.3V/5V según dispositivo)");
  Serial.println("   - Agregar resistencias pull-up 4.7kΩ");
  Serial.println("   - Cables I2C no deben exceder 30cm");
  Serial.println();
  
  Serial.println("🟡 Comunicación intermitente:");
  Serial.println("   - Reducir velocidad I2C a 100kHz");
  Serial.println("   - Verificar calidad de alimentación");
  Serial.println("   - Revisar interferencias electromagnéticas");
  Serial.println("   - Conectar capacitor 100nF en alimentación");
  Serial.println();
  
  Serial.println("🟢 Sistema funcionando correctamente:");
  Serial.println("   - PCA9685 en 0x40: Control de servos OK");
  Serial.println("   - OLED en 0x3C/0x3D: Display OK");
  Serial.println("   - Comunicación estable sin errores");
  Serial.println();
}

/*
===============================================================================
                                MANUAL DE USO
===============================================================================

🎯 OBJETIVO:
Diagnosticar y verificar la comunicación I2C entre el ESP32 y los dispositivos
del sistema UpCycle Pro (PCA9685 y OLED).

🔧 CONFIGURACIÓN:
1. Conectar dispositivos I2C:
   - SDA: GPIO21 (todos los dispositivos)
   - SCL: GPIO22 (todos los dispositivos)
   - OLED: Alimentación 3.3V
   - PCA9685: Alimentación 5V

2. Subir este sketch al ESP32
3. Abrir Monitor Serial (115200 baud)

📋 PROCEDIMIENTO DE VERIFICACIÓN:

ESCANEO AUTOMÁTICO:
1. Sistema escanea automáticamente cada 10 segundos
2. Reporta dispositivos encontrados con direcciones
3. Identifica dispositivos conocidos del sistema

COMANDOS MANUALES:
- 's': Escanear dispositivos inmediatamente
- 'a': Activar/desactivar escaneo automático
- 't': Probar comunicación con dispositivos encontrados
- 'v': Cambiar velocidad I2C (100kHz/400kHz/1MHz)
- 'r': Reiniciar bus I2C
- 'h': Mostrar diagnóstico completo

✅ RESULTADOS ESPERADOS:
Para un sistema UpCycle Pro funcionando correctamente:
- 0x40: PCA9685 (Controlador de servos)
- 0x3C o 0x3D: OLED SSD1306 (Display)
- Total: 2 dispositivos detectados

❌ PROBLEMAS Y SOLUCIONES:

"No se encontraron dispositivos":
- Verificar conexiones SDA (GPIO21) y SCL (GPIO22)
- Verificar alimentación de dispositivos
- Agregar resistencias pull-up 4.7kΩ en SDA y SCL
- Verificar que cables no excedan 30cm

"Solo se detecta un dispositivo":
- Verificar alimentación específica (3.3V para OLED, 5V para PCA9685)
- Verificar conexiones de dispositivo faltante
- Probar direcciones alternativas (0x3D para OLED)

"Comunicación intermitente":
- Reducir velocidad I2C con comando 'v' -> '1'
- Verificar calidad de alimentación
- Agregar capacitores de desacoplamiento (100nF)
- Verificar interferencias electromagnéticas

"Error desconocido en dirección X":
- Dispositivo presente pero no responde correctamente
- Verificar compatibilidad de voltaje
- Verificar que dispositivo no esté dañado

📊 INTERPRETACIÓN DE RESULTADOS:

✅ Sistema perfecto:
- PCA9685 en 0x40: Control de servos disponible
- OLED en 0x3C/0x3D: Display disponible
- Comunicación estable sin errores

⚠️ Sistema parcial:
- Solo un dispositivo detectado
- Verificar el dispositivo faltante específicamente

❌ Sistema no funcional:
- No hay dispositivos detectados
- Problema fundamental de I2C o conexiones

🔧 SIGUIENTE PASO:
Una vez que ambos dispositivos sean detectados correctamente,
proceder a probar cada uno individualmente con sus sketches específicos.

===============================================================================
*/