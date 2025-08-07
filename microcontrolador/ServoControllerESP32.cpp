#include "ServoControllerESP32.h"

// Constructor por defecto
ServoControllerESP32::ServoControllerESP32() {
  i2cAddress = 0x40;
  pwmFrequency = 80;
  pwmDriver = new Adafruit_PWMServoDriver(i2cAddress);
}

// Constructor con parámetros personalizados
ServoControllerESP32::ServoControllerESP32(uint8_t address, uint16_t frequency) {
  i2cAddress = address;
  pwmFrequency = frequency;
  pwmDriver = new Adafruit_PWMServoDriver(i2cAddress);
}

// Inicializar la librería
bool ServoControllerESP32::begin() {
  if (!pwmDriver) {
    return false;
  }
  
  pwmDriver->begin();
  pwmDriver->setPWMFreq(pwmFrequency);
  
  // Pequeña pausa para estabilización
  delay(10);
  
  return true;
}

// Configurar límites para servos de 180°
void ServoControllerESP32::setServo180Limits(uint16_t min_pulse, uint16_t max_pulse) {
  pos0_180 = min_pulse;
  pos180_180 = max_pulse;
}

// Configurar límites para servos de 360°
void ServoControllerESP32::setServo360Limits(uint16_t min_pulse, uint16_t max_pulse) {
  pos0_360 = min_pulse;
  pos360_360 = max_pulse;
}

// Función principal para mover servo
void ServoControllerESP32::moveServo(uint8_t servo_num, int angle, ServoType type) {
  if (!isValidAngle(angle, type)) {
    // Opcional: agregar debug o error handling
    return;
  }
  
  uint16_t pulse_width = angleToPulse(angle, type);
  pwmDriver->setPWM(servo_num, 0, pulse_width);
}

// Detener servo de rotación continua
void ServoControllerESP32::stopServo(uint8_t servo_num) {
  // Para servos de 360°, típicamente 90° es la posición neutral (detenido)
  uint16_t stop_pulse = map(90, 0, 360, pos0_360, pos360_360);
  pwmDriver->setPWM(servo_num, 0, stop_pulse);
}

// Establecer PWM directamente
void ServoControllerESP32::setPWM(uint8_t servo_num, uint16_t pulse_width) {
  pwmDriver->setPWM(servo_num, 0, pulse_width);
}

// Convertir ángulo a ancho de pulso
uint16_t ServoControllerESP32::angleToPulse(int angle, ServoType type) {
  uint16_t pulse_width;
  
  switch (type) {
    case SERVO_180:
      pulse_width = map(angle, 0, 180, pos0_180, pos180_180);
      break;
      
    case SERVO_360:
      pulse_width = map(angle, 0, 360, pos0_360, pos360_360);
      break;
      
    default:
      pulse_width = pos0_180; // Valor por defecto seguro
      break;
  }
  
  return pulse_width;
}

// Validar si el ángulo es válido para el tipo de servo
bool ServoControllerESP32::isValidAngle(int angle, ServoType type) {
  switch (type) {
    case SERVO_180:
      return (angle >= 0 && angle <= 180);
      
    case SERVO_360:
      return (angle >= 0 && angle <= 360);
      
    default:
      return false;
  }
}