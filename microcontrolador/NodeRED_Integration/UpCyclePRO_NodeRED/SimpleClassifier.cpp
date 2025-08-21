#include "SimpleClassifier.h"

SimpleClassifier::SimpleClassifier() {
  currentState = IDLE;
  selectedMode = 0;
  lastActivity = 0;
  
  // Inicializar peso diario
  weightLoggingEnabled = true;
  dailyWeightVidrio = 0.0f;
  dailyWeightPlastico = 0.0f;
  dailyWeightMetal = 0.0f;
  lastWeightReset = millis();
  
  // Inicializar ESP32 CAM + API
  apiPrediction = "";
  waitingForConfirmation = false;
  predictionTime = 0;
  confirmationTimeout = 10000; // 10 segundos para confirmar PIR
}

bool SimpleClassifier::begin() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("========================================");
  Serial.println("   CLASIFICADORA ESP32 CAM v2.0");
  Serial.println("========================================");
  
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
  
  // Inicializar sistema de peso para acumulado diario
  if (weightLoggingEnabled) {
    if (!weightSensors.begin()) {
      Serial.println("⚠️ Error inicializando celdas de carga - Continuando sin peso diario");
      weightLoggingEnabled = false;
    } else {
      Serial.println("✅ Sistema de peso inicializado (acumulado diario)");
    }
  }
  
  currentState = IDLE;
  
  Serial.println("✅ Sistema inicializado correctamente");
  Serial.println("📸 Modo: ESP32 CAM + API + PIR");
  Serial.println("📊 Peso diario: " + String(weightLoggingEnabled ? "HABILITADO" : "DESHABILITADO"));
  printHelp();
  
  return true;
}

void SimpleClassifier::startContinuousOperation() {
  currentState = BELT_RUNNING;
  lastActivity = millis();
  
  // Iniciar motor en modo continuo
  motor.start();
  
  Serial.println("🚀 OPERACIÓN CONTINUA INICIADA");
  Serial.println("📷 ESP32 CAM lista para detectar objetos");
  Serial.println("🔄 Banda transportadora corriendo continuamente");
  
  oledDisplay.showSystemStatus("BANDA ACTIVA");
}

void SimpleClassifier::processApiPrediction(String prediction) {
  if (currentState != BELT_RUNNING) {
    Serial.println("⚠️ Sistema no está en modo banda continua");
    return;
  }
  
  apiPrediction = prediction;
  waitingForConfirmation = true;
  predictionTime = millis();
  currentState = CAMERA_DETECTING;
  
  // Detener banda temporalmente
  motor.stop();
  
  Serial.println("📷 ESP32 CAM DETECTÓ OBJETO");
  Serial.println("🤖 Predicción API: " + prediction);
  Serial.println("⏸️ Banda detenida temporalmente");
  
  // Mostrar predicción en OLED
  oledDisplay.showSystemStatus("DETECTADO");
  delay(500);
  oledDisplay.showSystemStatus(prediction);
  
  // Reiniciar banda después de procesar
  delay(2000); // Tiempo para procesar
  motor.start();
  currentState = WAITING_FOR_PIR;
  
  Serial.println("🔄 Banda reiniciada - Esperando confirmación PIR");
  Serial.println("📡 OLED mostrando: " + prediction);
}

void SimpleClassifier::stopSystem() {
  motor.stop();
  servos.resetAllServos();
  currentState = IDLE;
  
  // Limpiar variables de API
  apiPrediction = "";
  waitingForConfirmation = false;
  predictionTime = 0;
  
  oledDisplay.showMainScreen(); // Volver a pantalla principal
  
  Serial.println("🛑 SISTEMA DETENIDO");
  Serial.println("📋 Listo para iniciar operación continua");
}

void SimpleClassifier::update() {
  // Actualizar display
  oledDisplay.update();
  
  // Verificar si hay que resetear pesos diarios (cada 24 horas)
  if (millis() - lastWeightReset > 86400000) { // 24 horas
    resetDailyWeights();
  }
  
  // Manejar estados según el flujo ESP32 CAM + API + PIR
  switch(currentState) {
    case IDLE:
      // Sistema inactivo, esperando comando para iniciar
      break;
      
    case BELT_RUNNING:
      // Banda corriendo continuamente, esperando detección de ESP32 CAM
      // Este estado se mantiene hasta que se llame processApiPrediction()
      break;
      
    case CAMERA_DETECTING:
      // ESP32 CAM procesando imagen, banda detenida temporalmente
      // Estado manejado en processApiPrediction()
      break;
      
    case WAITING_FOR_PIR:
      // Esperando confirmación PIR basada en predicción API
      
      // Verificar timeout de confirmación PIR
      if (millis() - predictionTime > confirmationTimeout) {
        Serial.println("⏰ Timeout PIR - Volviendo a banda continua");
        oledDisplay.showSystemStatus("TIMEOUT PIR");
        delay(1000);
        
        // Volver a banda continua
        currentState = BELT_RUNNING;
        waitingForConfirmation = false;
        apiPrediction = "";
        oledDisplay.showSystemStatus("BANDA ACTIVA");
        return;
      }
      
      // Verificar PIR según predicción de API
      if (checkPIRForPrediction()) {
        processConfirmedDetection();
      }
      break;
      
    case PROCESSING:
      // Servo activado, procesando material
      // Estado se cambia automáticamente en processConfirmedDetection()
      break;
  }
}

// Funciones auxiliares para el nuevo flujo

bool SimpleClassifier::checkPIRForPrediction() {
  // Verificar PIR según la predicción de la API
  if (apiPrediction == "VIDRIO" && pirSensors.checkPIR1()) {
    return true;
  } else if (apiPrediction == "PLASTICO" && pirSensors.checkPIR2()) {
    return true;
  } else if (apiPrediction == "METAL" && pirSensors.checkPIR3()) {
    return true;
  }
  return false;
}

void SimpleClassifier::processConfirmedDetection() {
  String material = apiPrediction;
  currentState = PROCESSING;
  
  // Detener banda
  motor.stop();
  
  Serial.println("✅ PIR CONFIRMÓ PREDICCIÓN: " + material);
  oledDisplay.showProcessing(material);
  
  // Activar servo correspondiente
  if (material == "VIDRIO") {
    servos.activateServo1();
  } else if (material == "PLASTICO") {
    servos.activateServo2();
  } else if (material == "METAL") {
    servos.activateServo3();
  }
  
  delay(1500); // Tiempo para completar empuje del servo
  
  // Leer peso para acumulado diario
  if (weightLoggingEnabled) {
    float weight1, weight2, weight3;
    weightSensors.readAllWeights(weight1, weight2, weight3);
    
    float objectWeight = 0.0f;
    if (material == "VIDRIO") objectWeight = weight1;
    else if (material == "PLASTICO") objectWeight = weight2;
    else if (material == "METAL") objectWeight = weight3;
    
    // Agregar al acumulado diario
    addDailyWeight(material, objectWeight);
    
    Serial.println("📊 PESO AGREGADO AL ACUMULADO DIARIO:");
    Serial.println("   Material: " + material);
    Serial.println("   Peso objeto: " + String(objectWeight, 3) + " kg");
    Serial.println("   Acumulado " + material + ": " + String(getDailyWeight(material), 3) + " kg");
  }
  
  // Completar procesamiento
  Serial.println("✅ CLASIFICACIÓN COMPLETADA");
  oledDisplay.showSystemStatus("COMPLETADO");
  delay(1000);
  
  // Volver a operación continua
  Serial.println("🔄 Volviendo a banda continua");
  currentState = BELT_RUNNING;
  waitingForConfirmation = false;
  apiPrediction = "";
  motor.start();
  oledDisplay.showSystemStatus("BANDA ACTIVA");
}

float SimpleClassifier::getDailyWeight(String material) {
  if (material == "VIDRIO") return dailyWeightVidrio;
  else if (material == "PLASTICO") return dailyWeightPlastico;
  else if (material == "METAL") return dailyWeightMetal;
  return 0.0f;
}

void SimpleClassifier::setMode(String material) {
  if (material == "vidrio") {
    selectedMode = 1;
  } else if (material == "plastico") {
    selectedMode = 2;
  } else if (material == "metal") {
    selectedMode = 3;
  } else {
    selectedMode = 0;
  }
  
  Serial.println("Modo seleccionado: " + material + " (" + String(selectedMode) + ")");
}

void SimpleClassifier::printStatus() {
  Serial.println("\n=== ESTADO DEL SISTEMA ===");
  
  String stateStr = "";
  switch(currentState) {
    case IDLE: stateStr = "INACTIVO"; break;
    case BELT_RUNNING: stateStr = "BANDA CONTINUA"; break;
    case CAMERA_DETECTING: stateStr = "CAM DETECTANDO"; break;
    case WAITING_FOR_PIR: stateStr = "ESPERANDO PIR"; break;
    case PROCESSING: stateStr = "PROCESANDO"; break;
  }
  
  String modeStr = "NINGUNO";
  switch(selectedMode) {
    case 1: modeStr = "1 - VIDRIO"; break;
    case 2: modeStr = "2 - PLASTICO"; break;
    case 3: modeStr = "3 - METAL"; break;
  }
  
  Serial.println("Estado: " + stateStr);
  Serial.println("Motor: " + String(motor.isMotorRunning() ? "FUNCIONANDO" : "DETENIDO"));
  Serial.println("Peso diario: " + String(weightLoggingEnabled ? "HABILITADO" : "DESHABILITADO"));
  
  if (waitingForConfirmation) {
    Serial.println("Predicción API: " + apiPrediction);
    unsigned long remaining = confirmationTimeout - (millis() - predictionTime);
    Serial.println("Timeout PIR: " + String(remaining/1000) + " segundos");
  }
  
  if (weightLoggingEnabled) {
    Serial.println("Pesos acumulados hoy:");
    Serial.println("  VIDRIO: " + String(dailyWeightVidrio, 3) + " kg");
    Serial.println("  PLASTICO: " + String(dailyWeightPlastico, 3) + " kg");
    Serial.println("  METAL: " + String(dailyWeightMetal, 3) + " kg");
  }
  
  Serial.println("===========================\n");
}

void SimpleClassifier::printHelp() {
  Serial.println("\n📋 COMANDOS DISPONIBLES:");
  Serial.println("start    - Iniciar operación continua");
  Serial.println("stop     - Detener sistema");
  Serial.println("status   - Ver estado del sistema");
  Serial.println("daily    - Ver pesos acumulados del día");
  Serial.println("reset    - Reiniciar pesos diarios");
  Serial.println("help     - Mostrar esta ayuda");
  Serial.println("");
  Serial.println("📸 FLUJO ESP32 CAM + API:");
  Serial.println("1. Banda corre continuamente");
  Serial.println("2. ESP32 CAM detecta objeto → llama API");
  Serial.println("3. API predice material");
  Serial.println("4. Banda se detiene, procesa, se reinicia");
  Serial.println("5. PIR correspondiente confirma material");
  Serial.println("6. Servo se activa y peso se acumula");
  Serial.println("===================================\n");
}

// Funciones para peso diario

void SimpleClassifier::enableDailyWeightLogging(bool enable) {
  weightLoggingEnabled = enable;
  Serial.println("📊 Peso diario: " + String(enable ? "HABILITADO" : "DESHABILITADO"));
  
  if (enable && !weightSensors.areAllScalesReady()) {
    Serial.println("⚠️ Advertencia: Algunas celdas de carga no están listas");
  }
}

void SimpleClassifier::resetDailyWeights() {
  dailyWeightVidrio = 0.0f;
  dailyWeightPlastico = 0.0f; 
  dailyWeightMetal = 0.0f;
  lastWeightReset = millis();
  
  Serial.println("🔄 Pesos diarios reiniciados");
  
  // TODO: Enviar datos acumulados a base de datos antes de reiniciar
  // sendDailyWeightsToDatabase(dailyWeightVidrio, dailyWeightPlastico, dailyWeightMetal);
}

void SimpleClassifier::addDailyWeight(String material, float weight) {
  if (!weightLoggingEnabled || weight <= 0) return;
  
  if (material == "VIDRIO") {
    dailyWeightVidrio += weight;
  } else if (material == "PLASTICO") {
    dailyWeightPlastico += weight;
  } else if (material == "METAL") {
    dailyWeightMetal += weight;
  }
}

void SimpleClassifier::printDailyWeights() {
  Serial.println("\n📊 PESOS ACUMULADOS HOY:");
  Serial.println("========================");
  Serial.println("VIDRIO:   " + String(dailyWeightVidrio, 3) + " kg");
  Serial.println("PLASTICO: " + String(dailyWeightPlastico, 3) + " kg");
  Serial.println("METAL:    " + String(dailyWeightMetal, 3) + " kg");
  Serial.println("TOTAL:    " + String(dailyWeightVidrio + dailyWeightPlastico + dailyWeightMetal, 3) + " kg");
  
  unsigned long hoursRunning = (millis() - lastWeightReset) / 3600000;
  Serial.println("Tiempo desde último reset: " + String(hoursRunning) + " horas");
  Serial.println("========================\n");
}