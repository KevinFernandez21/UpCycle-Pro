#ifndef CONVEYOR_MOTOR_H
#define CONVEYOR_MOTOR_H

#include <Arduino.h>

// Pin de control del motor
#define MOTOR_PIN 25

class ConveyorMotor {
private:
  bool isRunning;
  unsigned long startTime;
  
public:
  ConveyorMotor();
  void begin();
  void start();
  void stop();
  bool isMotorRunning() { return isRunning; }
  void printStatus();
};

#endif