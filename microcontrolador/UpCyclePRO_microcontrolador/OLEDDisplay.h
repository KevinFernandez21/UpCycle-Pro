#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configuración del display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Bitmap del ícono de reciclaje (24x24 píxeles)
static const unsigned char PROGMEM recycleBitmap[] = {
  0x00, 0x7E, 0x00, 0x01, 0xFF, 0x80, 0x03, 0xFF, 0xC0, 0x07, 0xE7, 0xE0,
  0x0F, 0xC3, 0xF0, 0x0F, 0x81, 0xF0, 0x1F, 0x00, 0xF8, 0x1E, 0x00, 0x78,
  0x3C, 0x18, 0x3C, 0x3C, 0x3C, 0x3C, 0x78, 0x7E, 0x1E, 0x78, 0xFF, 0x1E,
  0x70, 0xE7, 0x0E, 0xF1, 0xC3, 0x8F, 0xF3, 0x81, 0xCF, 0xE3, 0x00, 0xC7,
  0xE7, 0x00, 0xE7, 0xCF, 0x00, 0xF3, 0x9E, 0x00, 0x79, 0x3C, 0x00, 0x3C,
  0x78, 0x00, 0x1E, 0xF0, 0x00, 0x0F, 0xE0, 0x00, 0x07, 0xC0, 0x00, 0x03
};

class OLEDDisplay {
private:
  Adafruit_SSD1306 display;
  String currentMessage;
  unsigned long lastUpdate;
  bool showingMaterial;
  
  void drawRecycleIcon();
  void drawCenteredText(String text, int textSize, int y);
  
public:
  OLEDDisplay();
  bool begin();
  
  // Pantalla principal con ícono de reciclaje
  void showRecycleScreen();
  
  // Mostrar materiales detectados
  void showVidrio();
  void showPlastico(); 
  void showMetal();
  
  // Mostrar números de modo
  void showMode1(); // Modo 1 - VIDRIO
  void showMode2(); // Modo 2 - PLASTICO
  void showMode3(); // Modo 3 - METAL
  
  // Funciones de estado
  void showWeight(float weight);
  void showSystemStatus(String status);
  void showError(String error);
  void showWaiting(String material);
  void showProcessing(String material);
  
  // Control básico
  void clear();
  void update();
  
  // Volver a pantalla principal después de un tiempo
  void returnToMainScreen();
};

#endif