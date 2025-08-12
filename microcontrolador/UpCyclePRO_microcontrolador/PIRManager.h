#ifndef PIR_MANAGER_H
#define PIR_MANAGER_H

#include <Arduino.h>

// Pines de los sensores PIR
#define PIR1_PIN 26  // VIDRIO
#define PIR2_PIN 27  // PLASTICO
#define PIR3_PIN 14  // METAL

class PIRManager {
private:
  bool lastState1, lastState2, lastState3;
  unsigned long lastTrigger1, lastTrigger2, lastTrigger3;
  unsigned long debounceTime = 500; // 0.5 segundos entre detecciones
  
public:
  PIRManager();
  void begin();
  bool checkPIR1(); // Retorna true si PIR1 se activó
  bool checkPIR2(); // Retorna true si PIR2 se activó  
  bool checkPIR3(); // Retorna true si PIR3 se activó
  void printStatus();
};

#endif