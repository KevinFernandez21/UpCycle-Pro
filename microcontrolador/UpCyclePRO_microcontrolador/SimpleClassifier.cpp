#include "SimpleClassifier.h"

SimpleClassifier::SimpleClassifier() {
  currentState = IDLE;
  selectedMode = 0;
  lastActivity = 0;
}

bool SimpleClassifier::begin() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=====================================");
  Serial.println("   CLASIFICADORA SIMPLE v1.0");
  Serial.println("=====================================");
  
  // Inicializar componentes
  Serial.println("Inicializando componentes...");
  
  // Inicializar OLED primero
  if (!oledDisplay.begin()) {
    Serial.println("❌ Error inicializando OLED - Continuando sin display");
  } else {
    Serial.println("✅ Display OLED inicializado");
  }
  
  motor.begin();
  pirSensors.begin();
  
  if (!servos.begin()) {
    Serial.println("❌ Error inicializando servos");
    return false;
  }
  
  currentState = IDLE;
  
  Serial.println("✅ Sistema inicializado correctamente");
  printHelp();
  
  return true;
}

void SimpleClassifier::setMode(int mode) {
  if (mode < 1 || mode > 3) {
    Serial.println("❌ Modo inválido. Use 1, 2 o 3");
    oledDisplay.showError("Modo invalido");
    return;
  }
  
  selectedMode = mode;
  
  String material = "";
  switch(mode) {
    case 1: 
      material = "VIDRIO";
      oledDisplay.showMode1();
      break;
    case 2: 
      material = "PLASTICO";
      oledDisplay.showMode2();
      break;
    case 3: 
      material = "METAL";
      oledDisplay.showMode3();
      break;
  }
  
  Serial.println("✅ Modo seleccionado: " + String(mode) + " - " + material);
  Serial.println("📋 Sistema listo para clasificar " + material);
  
  delay(2000); // Mostrar modo por 2 segundos
  startClassification();
}

void SimpleClassifier::startClassification() {
  if (selectedMode == 0) {
    Serial.println("❌ No se ha seleccionado modo. Use 1, 2 o 3");
    oledDisplay.showError("Sin modo");
    return;
  }
  
  currentState = WAITING;
  lastActivity = millis();
  
  // Iniciar motor
  motor.start();
  
  String material = "";
  switch(selectedMode) {
    case 1: 
      material = "VIDRIO";
      oledDisplay.showWaiting("VIDRIO");
      break;
    case 2: 
      material = "PLASTICO";
      oledDisplay.showWaiting("PLASTICO");
      break;
    case 3: 
      material = "METAL";
      oledDisplay.showWaiting("METAL");
      break;
  }
  
  Serial.println("🚀 CLASIFICACIÓN INICIADA");
  Serial.println("📡 Esperando " + material);
  Serial.println("🔴 Motor en funcionamiento");
}

void SimpleClassifier::stopSystem() {
  motor.stop();
  servos.resetAllServos();
  currentState = IDLE;
  selectedMode = 0;
  
  oledDisplay.showMainScreen(); // Volver a pantalla principal
  
  Serial.println("🛑 SISTEMA DETENIDO");
  Serial.println("📋 Listo para nuevo comando");
}

void SimpleClassifier::update() {
  // Actualizar display
  oledDisplay.update();
  
  // Verificar timeout si está esperando
  if (currentState == WAITING) {
    if (millis() - lastActivity > timeout) {
      Serial.println("⏰ Timeout - Deteniendo sistema");
      oledDisplay.showSystemStatus("TIMEOUT");
      delay(2000);
      stopSystem();
      return;
    }
  }
  
  // Solo verificar sensores si está en modo espera
  if (currentState != WAITING) {
    return;
  }
  
  // Verificar sensores según el modo seleccionado
  bool detected = false;
  String material = "";
  
  switch(selectedMode) {
    case 1: // VIDRIO
      if (pirSensors.checkPIR1()) {
        Serial.println("🎯 VIDRIO DETECTADO - Activando Servo 1");
        material = "VIDRIO";
        currentState = PROCESSING;
        motor.stop();
        oledDisplay.showProcessing("VIDRIO");
        servos.activateServo1();
        detected = true;
      }
      break;
      
    case 2: // PLASTICO
      if (pirSensors.checkPIR2()) {
        Serial.println("🎯 PLASTICO DETECTADO - Activando Servo 2");
        material = "PLASTICO";
        currentState = PROCESSING;
        motor.stop();
        oledDisplay.showProcessing("PLASTICO");
        servos.activateServo2();
        detected = true;
      }
      break;
      
    case 3: // METAL
      if (pirSensors.checkPIR3()) {
        Serial.println("🎯 METAL DETECTADO - Activando Servo 3");
        material = "METAL";
        currentState = PROCESSING;
        motor.stop();
        oledDisplay.showProcessing("METAL");
        servos.activateServo3();
        detected = true;
      }
      break;
  }
  
  // Si se detectó algo, completar proceso
  if (detected) {
    delay(1500); // Tiempo para completar empuje
    
    Serial.println("✅ CLASIFICACIÓN COMPLETADA");
    Serial.println("📋 Listo para siguiente objeto");
    
    oledDisplay.showSystemStatus("COMPLETADO");
    delay(1000);
    
    // Volver a estado de espera con el mismo modo
    currentState = WAITING;
    lastActivity = millis();
    motor.start(); // Reiniciar motor
    oledDisplay.showWaiting(material);
  }
}

void SimpleClassifier::printStatus() {
  Serial.println("\n=== ESTADO DEL SISTEMA ===");
  
  String stateStr = "";
  switch(currentState) {
    case IDLE: stateStr = "INACTIVO"; break;
    case WAITING: stateStr = "ESPERANDO DETECCIÓN"; break;
    case PROCESSING: stateStr = "PROCESANDO"; break;
  }
  
  String modeStr = "NINGUNO";
  switch(selectedMode) {
    case 1: modeStr = "1 - VIDRIO"; break;
    case 2: modeStr = "2 - PLASTICO"; break;
    case 3: modeStr = "3 - METAL"; break;
  }
  
  Serial.println("Estado: " + stateStr);
  Serial.println("Modo: " + modeStr);
  Serial.println("Motor: " + String(motor.isMotorRunning() ? "FUNCIONANDO" : "DETENIDO"));
  
  if (currentState == WAITING) {
    unsigned long remaining = timeout - (millis() - lastActivity);
    Serial.println("Timeout en: " + String(remaining/1000) + " segundos");
  }
  
  Serial.println("===========================\n");
}

void SimpleClassifier::printHelp() {
  Serial.println("\n📋 COMANDOS DISPONIBLES:");
  Serial.println("1 - Clasificar VIDRIO (PIR1 → Servo1)");
  Serial.println("2 - Clasificar PLASTICO (PIR2 → Servo2)");
  Serial.println("3 - Clasificar METAL (PIR3 → Servo3)");
  Serial.println("stop - Detener sistema");
  Serial.println("status - Ver estado del sistema");
  Serial.println("help - Mostrar esta ayuda");
  Serial.println("===================================\n");
}