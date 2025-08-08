#include "ServoManager.h"

ServoManager::ServoManager() : pwm(0x40) {
  // Constructor con dirección I2C por defecto 0x40
}

bool ServoManager::begin() {
  Serial.println("Inicializando controlador de servos PCA9685...");
  
  pwm.begin();
  pwm.setPWMFreq(60);  // Frecuencia 60Hz para servos
  
  delay(10);
  
  // Poner todos los servos en posición de reposo
  resetAllServos();
  
  Serial.println("=== SERVOS INICIALIZADOS ===");
  Serial.println("Servo1 (Vidrio)   -> Canal " + String(SERVO1_CHANNEL));
  Serial.println("Servo2 (Plástico) -> Canal " + String(SERVO2_CHANNEL));
  Serial.println("Servo3 (Metal)    -> Canal " + String(SERVO3_CHANNEL));
  Serial.println("Posición reposo: " + String(restPosition) + "°");
  Serial.println("Posición empuje: " + String(pushPosition) + "°");
  Serial.println("Tiempo empuje: " + String(servoDelay) + " ms");
  Serial.println("============================");
  
  return true;
}

uint16_t ServoManager::angleToPulse(int angle) {
  // Convertir ángulo (0-180°) a valor PWM
  return map(angle, 0, 180, servoMin, servoMax);
}

void ServoManager::activateServo1() {
  Serial.println("🔵 ACTIVANDO SERVO 1 - Empujando VIDRIO");
  
  // Mover a posición de empuje
  uint16_t pulsePush = angleToPulse(pushPosition);
  pwm.setPWM(SERVO1_CHANNEL, 0, pulsePush);
  
  delay(servoDelay);
  
  // Volver a posición de reposo
  uint16_t pulseRest = angleToPulse(restPosition);
  pwm.setPWM(SERVO1_CHANNEL, 0, pulseRest);
  
  Serial.println("✅ SERVO 1 completado - VIDRIO empujado");
}

void ServoManager::activateServo2() {
  Serial.println("🟠 ACTIVANDO SERVO 2 - Empujando PLASTICO");
  
  // Mover a posición de empuje
  uint16_t pulsePush = angleToPulse(pushPosition);
  pwm.setPWM(SERVO2_CHANNEL, 0, pulsePush);
  
  delay(servoDelay);
  
  // Volver a posición de reposo
  uint16_t pulseRest = angleToPulse(restPosition);
  pwm.setPWM(SERVO2_CHANNEL, 0, pulseRest);
  
  Serial.println("✅ SERVO 2 completado - PLASTICO empujado");
}

void ServoManager::activateServo3() {
  Serial.println("⚫ ACTIVANDO SERVO 3 - Empujando METAL");
  
  // Mover a posición de empuje
  uint16_t pulsePush = angleToPulse(pushPosition);
  pwm.setPWM(SERVO3_CHANNEL, 0, pulsePush);
  
  delay(servoDelay);
  
  // Volver a posición de reposo
  uint16_t pulseRest = angleToPulse(restPosition);
  pwm.setPWM(SERVO3_CHANNEL, 0, pulseRest);
  
  Serial.println("✅ SERVO 3 completado - METAL empujado");
}

void ServoManager::resetAllServos() {
  Serial.println("🔄 Reseteando todos los servos a posición de reposo");
  
  uint16_t pulseRest = angleToPulse(restPosition);
  
  pwm.setPWM(SERVO1_CHANNEL, 0, pulseRest);
  pwm.setPWM(SERVO2_CHANNEL, 0, pulseRest);
  pwm.setPWM(SERVO3_CHANNEL, 0, pulseRest);
  
  delay(500);
  Serial.println("✅ Todos los servos en reposo");
}

void ServoManager::setPositions(int rest, int push) {
  restPosition = rest;
  pushPosition = push;
  Serial.println("Posiciones actualizadas - Reposo: " + String(rest) + "° Empuje: " + String(push) + "°");
}

void ServoManager::setDelay(int delayMs) {
  servoDelay = delayMs;
  Serial.println("Tiempo de empuje actualizado: " + String(delayMs) + " ms");
}

void ServoManager::printStatus() {
  Serial.println("=== ESTADO DE SERVOS ===");
  Serial.println("Servo1 (Vidrio):   Canal " + String(SERVO1_CHANNEL));
  Serial.println("Servo2 (Plástico): Canal " + String(SERVO2_CHANNEL));
  Serial.println("Servo3 (Metal):    Canal " + String(SERVO3_CHANNEL));
  Serial.println("Posición reposo:   " + String(restPosition) + "°");
  Serial.println("Posición empuje:   " + String(pushPosition) + "°");
  Serial.println("Tiempo empuje:     " + String(servoDelay) + " ms");
  Serial.println("========================");
}