#include "WeightManager.h"

WeightManager::WeightManager() {
  lastWeight1 = 0.0f;
  lastWeight2 = 0.0f;
  lastWeight3 = 0.0f;
  lastReading = 0;
  isCalibrated1 = false;
  isCalibrated2 = false;
  isCalibrated3 = false;
  bufferIndex = 0;
  
  // Inicializar buffers
  for (int i = 0; i < WEIGHT_SAMPLES; i++) {
    weightBuffer1[i] = 0.0f;
    weightBuffer2[i] = 0.0f;
    weightBuffer3[i] = 0.0f;
  }
}

bool WeightManager::begin() {
  Serial.println("🔄 Inicializando WeightManager...");
  
  // Inicializar escalas HX711
  scale1.begin(HX711_1_DOUT_PIN, HX711_1_SCK_PIN);
  scale2.begin(HX711_2_DOUT_PIN, HX711_2_SCK_PIN);
  scale3.begin(HX711_3_DOUT_PIN, HX711_3_SCK_PIN);
  
  delay(1000); // Tiempo de estabilización
  
  // Verificar que las celdas respondan
  if (!scale1.is_ready()) {
    Serial.println("❌ Error: Celda de carga 1 (VIDRIO) no responde");
    return false;
  }
  
  if (!scale2.is_ready()) {
    Serial.println("❌ Error: Celda de carga 2 (PLASTICO) no responde");
    return false;
  }
  
  if (!scale3.is_ready()) {
    Serial.println("❌ Error: Celda de carga 3 (METAL) no responde");
    return false;
  }
  
  // Configurar factores de escala predeterminados
  scale1.set_scale(SCALE_FACTOR_1);
  scale2.set_scale(SCALE_FACTOR_2);
  scale3.set_scale(SCALE_FACTOR_3);
  
  // Realizar tara automática
  Serial.println("🔄 Realizando tara automática...");
  tareAll();
  
  Serial.println("✅ WeightManager inicializado correctamente");
  Serial.println("📊 Celdas de carga:");
  Serial.println("   - Celda 1: VIDRIO (GPIO32/33)");
  Serial.println("   - Celda 2: PLASTICO (GPIO16/17)");
  Serial.println("   - Celda 3: METAL (GPIO18/19)");
  
  return true;
}

void WeightManager::calibrateScale1(float knownWeight) {
  if (!scale1.is_ready()) return;
  
  Serial.println("🔧 Calibrando celda 1 (VIDRIO) con peso: " + String(knownWeight) + "kg");
  
  scale1.set_scale();
  scale1.tare();
  
  Serial.println("📦 Coloca el peso conocido y presiona Enter...");
  while (!Serial.available()) delay(100);
  Serial.readString(); // Limpiar buffer
  
  float reading = scale1.get_units(10);
  float factor = reading / knownWeight;
  scale1.set_scale(factor);
  
  Serial.println("✅ Celda 1 calibrada. Factor: " + String(factor));
  isCalibrated1 = true;
}

void WeightManager::calibrateScale2(float knownWeight) {
  if (!scale2.is_ready()) return;
  
  Serial.println("🔧 Calibrando celda 2 (PLASTICO) con peso: " + String(knownWeight) + "kg");
  
  scale2.set_scale();
  scale2.tare();
  
  Serial.println("📦 Coloca el peso conocido y presiona Enter...");
  while (!Serial.available()) delay(100);
  Serial.readString(); // Limpiar buffer
  
  float reading = scale2.get_units(10);
  float factor = reading / knownWeight;
  scale2.set_scale(factor);
  
  Serial.println("✅ Celda 2 calibrada. Factor: " + String(factor));
  isCalibrated2 = true;
}

void WeightManager::calibrateScale3(float knownWeight) {
  if (!scale3.is_ready()) return;
  
  Serial.println("🔧 Calibrando celda 3 (METAL) con peso: " + String(knownWeight) + "kg");
  
  scale3.set_scale();
  scale3.tare();
  
  Serial.println("📦 Coloca el peso conocido y presiona Enter...");
  while (!Serial.available()) delay(100);
  Serial.readString(); // Limpiar buffer
  
  float reading = scale3.get_units(10);
  float factor = reading / knownWeight;
  scale3.set_scale(factor);
  
  Serial.println("✅ Celda 3 calibrada. Factor: " + String(factor));
  isCalibrated3 = true;
}

void WeightManager::tareAll() {
  Serial.println("⚖️ Realizando tara de todas las celdas...");
  
  if (scale1.is_ready()) {
    scale1.tare();
    Serial.println("✅ Tara celda 1 (VIDRIO) completada");
  }
  
  if (scale2.is_ready()) {
    scale2.tare();
    Serial.println("✅ Tara celda 2 (PLASTICO) completada");
  }
  
  if (scale3.is_ready()) {
    scale3.tare();
    Serial.println("✅ Tara celda 3 (METAL) completada");
  }
  
  delay(500); // Estabilización
}

void WeightManager::tareScale1() {
  if (scale1.is_ready()) {
    scale1.tare();
    Serial.println("✅ Tara celda 1 (VIDRIO) completada");
  }
}

void WeightManager::tareScale2() {
  if (scale2.is_ready()) {
    scale2.tare();
    Serial.println("✅ Tara celda 2 (PLASTICO) completada");
  }
}

void WeightManager::tareScale3() {
  if (scale3.is_ready()) {
    scale3.tare();
    Serial.println("✅ Tara celda 3 (METAL) completada");
  }
}

float WeightManager::readWeight1(bool filtered) {
  if (!scale1.is_ready()) return 0.0f;
  
  float weight = scale1.get_units(3); // Promedio de 3 lecturas
  
  if (filtered) {
    // Agregar al buffer circular
    weightBuffer1[bufferIndex] = weight;
    weight = calculateAverage(weightBuffer1);
  }
  
  lastWeight1 = weight;
  return weight;
}

float WeightManager::readWeight2(bool filtered) {
  if (!scale2.is_ready()) return 0.0f;
  
  float weight = scale2.get_units(3); // Promedio de 3 lecturas
  
  if (filtered) {
    // Agregar al buffer circular
    weightBuffer2[bufferIndex] = weight;
    weight = calculateAverage(weightBuffer2);
  }
  
  lastWeight2 = weight;
  return weight;
}

float WeightManager::readWeight3(bool filtered) {
  if (!scale3.is_ready()) return 0.0f;
  
  float weight = scale3.get_units(3); // Promedio de 3 lecturas
  
  if (filtered) {
    // Agregar al buffer circular
    weightBuffer3[bufferIndex] = weight;
    weight = calculateAverage(weightBuffer3);
  }
  
  lastWeight3 = weight;
  return weight;
}

void WeightManager::readAllWeights(float &weight1, float &weight2, float &weight3) {
  weight1 = readWeight1(true);
  weight2 = readWeight2(true);
  weight3 = readWeight3(true);
  
  // Actualizar índice del buffer circular
  bufferIndex = (bufferIndex + 1) % WEIGHT_SAMPLES;
  lastReading = millis();
}

bool WeightManager::isValidVidrioWeight(float weight) {
  return (weight >= VIDRIO_MIN_WEIGHT && weight <= VIDRIO_MAX_WEIGHT);
}

bool WeightManager::isValidPlasticoWeight(float weight) {
  return (weight >= PLASTICO_MIN_WEIGHT && weight <= PLASTICO_MAX_WEIGHT);
}

bool WeightManager::isValidMetalWeight(float weight) {
  return (weight >= METAL_MIN_WEIGHT && weight <= METAL_MAX_WEIGHT);
}

bool WeightManager::hasObjectOnScale1() {
  float weight = readWeight1(true);
  return (abs(weight) > MIN_WEIGHT);
}

bool WeightManager::hasObjectOnScale2() {
  float weight = readWeight2(true);
  return (abs(weight) > MIN_WEIGHT);
}

bool WeightManager::hasObjectOnScale3() {
  float weight = readWeight3(true);
  return (abs(weight) > MIN_WEIGHT);
}

bool WeightManager::hasAnyObject() {
  return (hasObjectOnScale1() || hasObjectOnScale2() || hasObjectOnScale3());
}

bool WeightManager::isScale1Ready() {
  return scale1.is_ready();
}

bool WeightManager::isScale2Ready() {
  return scale2.is_ready();
}

bool WeightManager::isScale3Ready() {
  return scale3.is_ready();
}

bool WeightManager::areAllScalesReady() {
  return (isScale1Ready() && isScale2Ready() && isScale3Ready());
}

void WeightManager::printWeights() {
  float w1, w2, w3;
  readAllWeights(w1, w2, w3);
  
  Serial.println("⚖️ PESOS ACTUALES:");
  Serial.println("   Celda 1 (VIDRIO): " + String(w1, 3) + " kg");
  Serial.println("   Celda 2 (PLASTICO): " + String(w2, 3) + " kg");
  Serial.println("   Celda 3 (METAL): " + String(w3, 3) + " kg");
}

void WeightManager::printCalibrationInfo() {
  Serial.println("🔧 INFORMACIÓN DE CALIBRACIÓN:");
  Serial.println("   Celda 1: " + String(isCalibrated1 ? "CALIBRADA" : "NO CALIBRADA"));
  Serial.println("   Celda 2: " + String(isCalibrated2 ? "CALIBRADA" : "NO CALIBRADA"));
  Serial.println("   Celda 3: " + String(isCalibrated3 ? "CALIBRADA" : "NO CALIBRADA"));
  
  Serial.println("📊 FACTORES DE ESCALA:");
  Serial.println("   Celda 1: " + String(scale1.get_scale()));
  Serial.println("   Celda 2: " + String(scale2.get_scale()));
  Serial.println("   Celda 3: " + String(scale3.get_scale()));
}

void WeightManager::printStatus() {
  Serial.println("📋 ESTADO DE CELDAS DE CARGA:");
  Serial.println("   Celda 1 (VIDRIO): " + String(isScale1Ready() ? "LISTO" : "ERROR"));
  Serial.println("   Celda 2 (PLASTICO): " + String(isScale2Ready() ? "LISTO" : "ERROR"));
  Serial.println("   Celda 3 (METAL): " + String(isScale3Ready() ? "LISTO" : "ERROR"));
  
  Serial.println("📦 DETECCIÓN DE OBJETOS:");
  Serial.println("   En celda 1: " + String(hasObjectOnScale1() ? "SÍ" : "NO"));
  Serial.println("   En celda 2: " + String(hasObjectOnScale2() ? "SÍ" : "NO"));
  Serial.println("   En celda 3: " + String(hasObjectOnScale3() ? "SÍ" : "NO"));
  
  printWeights();
}

void WeightManager::setScale1Factor(float factor) {
  scale1.set_scale(factor);
  Serial.println("🔧 Factor celda 1 actualizado: " + String(factor));
}

void WeightManager::setScale2Factor(float factor) {
  scale2.set_scale(factor);
  Serial.println("🔧 Factor celda 2 actualizado: " + String(factor));
}

void WeightManager::setScale3Factor(float factor) {
  scale3.set_scale(factor);
  Serial.println("🔧 Factor celda 3 actualizado: " + String(factor));
}

void WeightManager::powerDown() {
  scale1.power_down();
  scale2.power_down();
  scale3.power_down();
  Serial.println("💤 Celdas de carga en modo ahorro de energía");
}

void WeightManager::powerUp() {
  scale1.power_up();
  scale2.power_up();
  scale3.power_up();
  Serial.println("⚡ Celdas de carga activadas");
}

String WeightManager::predictMaterial(float weight) {
  if (isValidVidrioWeight(weight)) {
    return "VIDRIO";
  } else if (isValidPlasticoWeight(weight)) {
    return "PLASTICO";
  } else if (isValidMetalWeight(weight)) {
    return "METAL";
  } else {
    return "DESCONOCIDO";
  }
}

int WeightManager::getMaterialByWeight(float weight1, float weight2, float weight3) {
  // Encontrar la celda con mayor peso (objeto detectado)
  float maxWeight = max(max(weight1, weight2), weight3);
  
  if (maxWeight < MIN_WEIGHT) {
    return 0; // No hay objeto significativo
  }
  
  // Determinar cuál celda tiene el objeto
  if (weight1 >= maxWeight && weight1 > MIN_WEIGHT) {
    return isValidVidrioWeight(weight1) ? 1 : 0;
  } else if (weight2 >= maxWeight && weight2 > MIN_WEIGHT) {
    return isValidPlasticoWeight(weight2) ? 2 : 0;
  } else if (weight3 >= maxWeight && weight3 > MIN_WEIGHT) {
    return isValidMetalWeight(weight3) ? 3 : 0;
  }
  
  return 0; // No se pudo determinar material
}

float WeightManager::calculateAverage(float buffer[]) {
  float sum = 0.0f;
  for (int i = 0; i < WEIGHT_SAMPLES; i++) {
    sum += buffer[i];
  }
  return sum / WEIGHT_SAMPLES;
}

bool WeightManager::isWeightStable(float currentWeight, float lastWeight) {
  float difference = abs(currentWeight - lastWeight);
  return (difference < 0.01f); // Estable si diferencia < 10g
}