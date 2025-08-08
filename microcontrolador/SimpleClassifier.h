#ifndef SIMPLE_CLASSIFIER_H
#define SIMPLE_CLASSIFIER_H

#include <Arduino.h>
#include "ConveyorMotor.h"
#include "PIRManager.h"
#include "ServoManager.h"
#include "OLEDDisplay.h"  // ← Agregamos OLED

// Estados del sistema
enum SystemState {
  IDLE,           // Sistema inactivo
  WAITING,        // Esperando detección
  PROCESSING      // Procesando material
};

class SimpleClassifier {
private:
  ConveyorMotor motor;
  PIRManager pirSensors;
  ServoManager servos;
  OLEDDisplay oledDisplay;  // ← Agregamos OLED
  
  SystemState currentState;
  int selectedMode;  // 1=Vidrio, 2=Plástico, 3=Metal, 0=Ninguno
  unsigned long lastActivity;
  unsigned long timeout = 30000;  // 30 segundos timeout
  
public:
  SimpleClassifier();
  bool begin();
  
  // Control principal
  void setMode(int mode);  // 1, 2 o 3
  void startClassification();
  void stopSystem();
  void update();  // Llamar en loop()
  
  // Estado
  SystemState getState() { return currentState; }
  int getMode() { return selectedMode; }
  
  // Utilidades
  void printStatus();
  void printHelp();
};

#endif