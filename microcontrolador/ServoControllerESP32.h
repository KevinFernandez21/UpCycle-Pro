#ifndef SERVO_CONTROLLER_ESP32_H
#define SERVO_CONTROLLER_ESP32_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Tipos de servo disponibles
enum ServoType {
  SERVO_180,    // Servo estándar 0-180 grados
  SERVO_360     // Servo de rotación continua 0-360 grados
};

class ServoControllerESP32 {
private:
  Adafruit_PWMServoDriver* pwmDriver;
  uint8_t i2cAddress;
  uint16_t pwmFrequency;
  
  // Valores por defecto para servos de 180°
  uint16_t pos0_180 = 172;    // Pulso para 0° en servos 180°
  uint16_t pos180_180 = 565;  // Pulso para 180° en servos 180°
  
  // Valores por defecto para servos de 360°
  uint16_t pos0_360 = 172;    // Pulso para 0° en servos 360°
  uint16_t pos360_360 = 565;  // Pulso para 360° en servos 360°
  
public:
  // Constructor por defecto (dirección I2C 0x40, frecuencia 60Hz)
  ServoControllerESP32();
  
  // Constructor con parámetros personalizados
  ServoControllerESP32(uint8_t address, uint16_t frequency = 60);
  
  // Inicializar la librería
  bool begin();
  
  // Configurar valores de pulso para servos de 180°
  void setServo180Limits(uint16_t min_pulse, uint16_t max_pulse);
  
  // Configurar valores de pulso para servos de 360°
  void setServo360Limits(uint16_t min_pulse, uint16_t max_pulse);
  
  // Mover servo a un ángulo específico
  void moveServo(uint8_t servo_num, int angle, ServoType type = SERVO_180);
  
  // Detener servo de rotación continua (solo para SERVO_360)
  void stopServo(uint8_t servo_num);
  
  // Configuración avanzada: establecer PWM directamente
  void setPWM(uint8_t servo_num, uint16_t pulse_width);
  
  // Obtener valores de configuración actuales
  uint16_t getServo180Min() { return pos0_180; }
  uint16_t getServo180Max() { return pos180_180; }
  uint16_t getServo360Min() { return pos0_360; }
  uint16_t getServo360Max() { return pos360_360; }
  
  // Utilidades
  uint16_t angleToPulse(int angle, ServoType type);
  bool isValidAngle(int angle, ServoType type);
};

#endif