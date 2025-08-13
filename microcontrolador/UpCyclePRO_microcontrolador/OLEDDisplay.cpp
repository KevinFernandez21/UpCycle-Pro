#include "OLEDDisplay.h"

OLEDDisplay::OLEDDisplay() : 
  display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
  currentMessage(""),
  lastUpdate(0),
  showingMaterial(false) {
}

bool OLEDDisplay::begin() {
  Wire.begin(21, 22);
  Wire.setClock(100000); // Reducir velocidad I2C para mayor estabilidad
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Error: No se pudo inicializar el display OLED");
    Serial.println("Verificar conexiones I2C: SDA=21, SCL=22");
    Serial.println("Verificar alimentación OLED: 3.3V");
    return false;
  }
  
  display.clearDisplay();
  showMainScreen(); // Mostrar pantalla principal UpCyclePro
  Serial.println("Display OLED inicializado correctamente");
  return true;
}

void OLEDDisplay::showMainScreen() {
  display.clearDisplay();
  
  // Título "UpCyclePro" centrado en la parte superior
  drawCenteredText("UpCyclePro", 2, 0);
  
  // Subtítulo "LISTO" en la parte inferior
  drawCenteredText("LISTO", 1, 24);
  
  display.display();
  showingMaterial = false;
  currentMessage = "UpCyclePro";
  lastUpdate = millis();
  
  Serial.println("Display: Pantalla principal - UpCyclePro");
}

void OLEDDisplay::drawRecycleIcon() {
  // Centrar el ícono de 24x24 en la pantalla de 128x32
  int iconX = (SCREEN_WIDTH - 24) / 2;  // Centrado horizontalmente
  int iconY = 4;  // Pequeño margen superior para pantalla de 32px
  
  display.drawBitmap(iconX, iconY, recycleBitmap, 24, 24, SSD1306_WHITE);
}

void OLEDDisplay::drawCenteredText(String text, int textSize, int y) {
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);
  
  // Calcular posición X para centrar el texto
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  int x = (SCREEN_WIDTH - w) / 2;
  
  display.setCursor(x, y);
  display.println(text);
}

void OLEDDisplay::showMode1() {
  display.clearDisplay();
  
  // Número grande optimizado para 32px altura
  drawCenteredText("1", 2, 0);
  
  // Material
  drawCenteredText("VIDRIO", 1, 20);
  
  display.display();
  showingMaterial = true;
  currentMessage = "MODO 1 - VIDRIO";
  lastUpdate = millis();
  
  Serial.println("Display: Modo 1 - VIDRIO");
}

void OLEDDisplay::showMode2() {
  display.clearDisplay();
  
  // Número grande optimizado para 32px altura
  drawCenteredText("2", 2, 0);
  
  // Material
  drawCenteredText("PLASTICO", 1, 20);
  
  display.display();
  showingMaterial = true;
  currentMessage = "MODO 2 - PLASTICO";
  lastUpdate = millis();
  
  Serial.println("Display: Modo 2 - PLASTICO");
}

void OLEDDisplay::showMode3() {
  display.clearDisplay();
  
  // Número grande optimizado para 32px altura
  drawCenteredText("3", 2, 0);
  
  // Material
  drawCenteredText("METAL", 1, 20);
  
  display.display();
  showingMaterial = true;
  currentMessage = "MODO 3 - METAL";
  lastUpdate = millis();
  
  Serial.println("Display: Modo 3 - METAL");
}

void OLEDDisplay::showVidrio() {
  display.clearDisplay();
  
  // Título compacto para 32px
  drawCenteredText("MATERIAL:", 1, 0);
  
  // Material en grande pero ajustado
  drawCenteredText("VIDRIO", 1, 16);
  
  display.display();
  showingMaterial = true;
  currentMessage = "VIDRIO";
  lastUpdate = millis();
  
  Serial.println("Display: Mostrando VIDRIO");
}

void OLEDDisplay::showPlastico() {
  display.clearDisplay();
  
  // Título compacto para 32px
  drawCenteredText("MATERIAL:", 1, 0);
  
  // Material en grande pero ajustado
  drawCenteredText("PLASTICO", 1, 16);
  
  display.display();
  showingMaterial = true;
  currentMessage = "PLASTICO";
  lastUpdate = millis();
  
  Serial.println("Display: Mostrando PLASTICO");
}

void OLEDDisplay::showMetal() {
  display.clearDisplay();
  
  // Título compacto para 32px
  drawCenteredText("MATERIAL:", 1, 0);
  
  // Material en grande pero ajustado
  drawCenteredText("METAL", 1, 16);
  
  display.display();
  showingMaterial = true;
  currentMessage = "METAL";
  lastUpdate = millis();
  
  Serial.println("Display: Mostrando METAL");
}

void OLEDDisplay::showWaiting(String material) {
  display.clearDisplay();
  
  drawCenteredText("ESPERANDO:", 1, 0);
  drawCenteredText(material, 2, 12);
  
  display.display();
  currentMessage = "ESPERANDO " + material;
  lastUpdate = millis();
  
  Serial.println("Display: Esperando " + material);
}

void OLEDDisplay::showProcessing(String material) {
  display.clearDisplay();
  
  drawCenteredText("PROCESANDO:", 1, 0);
  drawCenteredText(material, 2, 12);
  
  display.display();
  currentMessage = "PROCESANDO " + material;
  lastUpdate = millis();
  
  Serial.println("Display: Procesando " + material);
}

void OLEDDisplay::showWeight(float weight) {
  display.clearDisplay();
  
  drawCenteredText("PESO:", 1, 0);
  
  // Mostrar peso con 1 decimal, ajustado para 32px
  String weightText = String(weight, 1) + " g";
  drawCenteredText(weightText, 1, 16);
  
  display.display();
  currentMessage = "PESO: " + weightText;
  lastUpdate = millis();
  
  Serial.println("Display: Mostrando peso - " + weightText);
}

void OLEDDisplay::showSystemStatus(String status) {
  display.clearDisplay();
  
  drawCenteredText("ESTADO:", 1, 0);
  drawCenteredText(status, 1, 16);
  
  display.display();
  currentMessage = status;
  lastUpdate = millis();
  
  Serial.println("Display: Estado - " + status);
}

void OLEDDisplay::showError(String error) {
  display.clearDisplay();
  
  drawCenteredText("ERROR:", 1, 0);
  drawCenteredText(error, 1, 16);
  
  display.display();
  currentMessage = "ERROR: " + error;
  lastUpdate = millis();
  
  Serial.println("Display: Error - " + error);
}

void OLEDDisplay::clear() {
  display.clearDisplay();
  display.display();
  currentMessage = "";
}

void OLEDDisplay::update() {
  // Si está mostrando un material por más de 3 segundos, volver a pantalla principal
  if (showingMaterial && (millis() - lastUpdate > 3000)) {
    showMainScreen();
  }
}

void OLEDDisplay::returnToMainScreen() {
  showMainScreen();
}