#include "ConveyorMotor.h"

ConveyorMotor::ConveyorMotor() {
  isRunning = false;
  startTime = 0;
}

void ConveyorMotor::begin() {
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);
  isRunning = false;
  
  Serial.println("Motor inicializado en pin " + String(MOTOR_PIN));
  Serial.println("Transistor: 2N2222 con resistencia 220Ω");
}

void ConveyorMotor::start() {
  if (isRunning) {
    Serial.println("Motor ya está funcionando");
    return;
  }
  
  digitalWrite(MOTOR_PIN, HIGH);
  isRunning = true;
  startTime = millis();
  
  Serial.println("🔴 MOTOR INICIADO - Banda en movimiento");
}

void ConveyorMotor::stop() {
  if (!isRunning) {
    Serial.println("Motor ya está detenido");
    return;
  }
  
  digitalWrite(MOTOR_PIN, LOW);
  isRunning = false;
  
  unsigned long runTime = millis() - startTime;
  Serial.println("🟢 MOTOR DETENIDO - Funcionó por " + String(runTime/1000.0) + " segundos");
}

void ConveyorMotor::printStatus() {
  Serial.println("=== ESTADO DEL MOTOR ===");
  Serial.println("Estado: " + String(isRunning ? "FUNCIONANDO" : "DETENIDO"));
  if (isRunning) {
    unsigned long currentRunTime = millis() - startTime;
    Serial.println("Tiempo funcionando: " + String(currentRunTime/1000.0) + " segundos");
  }
  Serial.println("========================");
}