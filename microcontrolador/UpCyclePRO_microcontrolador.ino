#include "ServoControllerESP32.h"

// Crear instancia del controlador de servos
ServoControllerESP32 servoController;

void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando controlador de servos...");
  
  // Inicializar la librería
  if (!servoController.begin()) {
    Serial.println("Error: No se pudo inicializar el controlador de servos");
    while(1); // Detener ejecución si hay error
  }
  
  Serial.println("Controlador inicializado correctamente");
  
  // CONFIGURACIÓN OPCIONAL: Ajustar límites de pulso si es necesario
  // Para servos de 180° (valores por defecto: 172, 565)
  // servoController.setServo180Limits(150, 600);
  
  // Para servos de 360° (valores por defecto: 172, 565)  
  // servoController.setServo360Limits(150, 600);
  
  Serial.println("Comenzando secuencia de movimientos...");
}

void loop() {
  // ========== EJEMPLO CON SERVOS DE 180° ==========
  Serial.println("Moviendo servos de 180°...");
  
  // Servo 0: mover a 30°
  servoController.moveServo(0, 0, SERVO_180);
  Serial.println("Servo 0 -> 30°");
  delay(1000);
  
  // Servo 0: mover a 180°
  servoController.moveServo(0, 180, SERVO_180);
  Serial.println("Servo 0 -> 180°");
  delay(1000);

  
  Serial.println("Ciclo completado. Reiniciando...\n");
}