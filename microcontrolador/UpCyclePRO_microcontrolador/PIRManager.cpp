#include "PIRManager.h"

PIRManager::PIRManager() {
  lastState1 = false;
  lastState2 = false;
  lastState3 = false;
  lastTrigger1 = 0;
  lastTrigger2 = 0;
  lastTrigger3 = 0;
}

void PIRManager::begin() {
  pinMode(PIR1_PIN, INPUT);
  pinMode(PIR2_PIN, INPUT);
  pinMode(PIR3_PIN, INPUT);
  
  Serial.println("=== SENSORES PIR INICIALIZADOS ===");
  Serial.println("PIR1 (Vidrio)   -> Pin " + String(PIR1_PIN));
  Serial.println("PIR2 (Plástico) -> Pin " + String(PIR2_PIN));
  Serial.println("PIR3 (Metal)    -> Pin " + String(PIR3_PIN));
  Serial.println("===================================");
  
  // Pausa para estabilización de sensores
  delay(2000);
  Serial.println("Sensores PIR listos");
}

bool PIRManager::checkPIR1() {
  bool currentState = digitalRead(PIR1_PIN);
  
  // Detectar flanco ascendente (activación)
  if (currentState && !lastState1) {
    // Verificar debounce
    if (millis() - lastTrigger1 > debounceTime) {
      lastTrigger1 = millis();
      lastState1 = currentState;
      Serial.println("🟡 PIR1 ACTIVADO - VIDRIO detectado");
      return true;
    }
  }
  
  lastState1 = currentState;
  return false;
}

bool PIRManager::checkPIR2() {
  bool currentState = digitalRead(PIR2_PIN);
  
  // Detectar flanco ascendente (activación)
  if (currentState && !lastState2) {
    // Verificar debounce
    if (millis() - lastTrigger2 > debounceTime) {
      lastTrigger2 = millis();
      lastState2 = currentState;
      Serial.println("🟡 PIR2 ACTIVADO - PLASTICO detectado");
      return true;
    }
  }
  
  lastState2 = currentState;
  return false;
}

bool PIRManager::checkPIR3() {
  bool currentState = digitalRead(PIR3_PIN);
  
  // Detectar flanco ascendente (activación)
  if (currentState && !lastState3) {
    // Verificar debounce
    if (millis() - lastTrigger3 > debounceTime) {
      lastTrigger3 = millis();
      lastState3 = currentState;
      Serial.println("🟡 PIR3 ACTIVADO - METAL detectado");
      return true;
    }
  }
  
  lastState3 = currentState;
  return false;
}

void PIRManager::printStatus() {
  bool state1 = digitalRead(PIR1_PIN);
  bool state2 = digitalRead(PIR2_PIN);
  bool state3 = digitalRead(PIR3_PIN);
  ¿
  Serial.println("=== ESTADO SENSORES PIR ===");
  Serial.println("PIR1 (Vidrio):   " + String(state1 ? "🔴 ACTIVO" : "⚫ INACTIVO"));
  Serial.println("PIR2 (Plástico): " + String(state2 ? "🔴 ACTIVO" : "⚫ INACTIVO"));
  Serial.println("PIR3 (Metal):    " + String(state3 ? "🔴 ACTIVO" : "⚫ INACTIVO"));
  Serial.println("===========================");
}