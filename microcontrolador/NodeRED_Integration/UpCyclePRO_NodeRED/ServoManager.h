#ifndef SERVO_MANAGER_H
#define SERVO_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Canales del PCA9685 para cada servo
#define SERVO1_CHANNEL 0  // VIDRIO
#define SERVO2_CHANNEL 1  // PLASTICO
#define SERVO3_CHANNEL 7  // METAL

class ServoManager {
private:
  Adafruit_PWMServoDriver pwm;
  
  // Posiciones de servos
  int restPosition = 90;     // Posición de reposo (inactivo)
  int pushPosition = 0;      // Posición de empuje (activo)
  int servoDelay = 1000;     // Tiempo de empuje en ms
  
  // Valores PWM para servos (ajustables)
  uint16_t servoMin = 172;   // Pulso mínimo
  uint16_t servoMax = 565;   // Pulso máximo
  
  uint16_t angleToPulse(int angle);
  
public:
  ServoManager();
  bool begin();
  void activateServo1(); // Empujar VIDRIO
  void activateServo2(); // Empujar PLASTICO
  void activateServo3(); // Empujar METAL  
  void resetAllServos(); // Todos a posición de reposo
  void disableAllServos(); // Apagar completamente los servos
  void printStatus();
  
  // Configuración
  void setPositions(int rest, int push);
  void setDelay(int delayMs);
};

#endif