#ifndef WEIGHT_MANAGER_H
#define WEIGHT_MANAGER_H

#include <Arduino.h>
#include "HX711.h"

// Configuración de pines para los 3 módulos HX711
#define HX711_1_DOUT_PIN  2  // Celda VIDRIO
#define HX711_1_SCK_PIN   4
#define HX711_2_DOUT_PIN  16  // Celda PLASTICO  
#define HX711_2_SCK_PIN   17
#define HX711_3_DOUT_PIN  5  // Celda METAL
#define HX711_3_SCK_PIN   19

// Factores de calibración (ajustar según calibración real)
#define SCALE_FACTOR_1   -7050.0f  // Factor para celda 1 (VIDRIO)
#define SCALE_FACTOR_2   -7050.0f  // Factor para celda 2 (PLASTICO)
#define SCALE_FACTOR_3   -7050.0f  // Factor para celda 3 (METAL)

// Offsets de tara (se calculan automáticamente)
#define TARE_OFFSET_1    0
#define TARE_OFFSET_2    0  
#define TARE_OFFSET_3    0

// Filtros y umbrales
#define WEIGHT_SAMPLES   5     // Número de muestras para promedio
#define MIN_WEIGHT      0.05f  // Peso mínimo detectado (50g)
#define MAX_WEIGHT     50.0f   // Peso máximo (50kg)
#define WEIGHT_TIMEOUT  5000   // Timeout para estabilización (5s)

// Rangos de peso por material (en kg)
#define VIDRIO_MIN_WEIGHT    0.1f   // 100g mínimo
#define VIDRIO_MAX_WEIGHT   15.0f   // 15kg máximo
#define PLASTICO_MIN_WEIGHT  0.05f  // 50g mínimo  
#define PLASTICO_MAX_WEIGHT  5.0f   // 5kg máximo
#define METAL_MIN_WEIGHT     0.2f   // 200g mínimo
#define METAL_MAX_WEIGHT    30.0f   // 30kg máximo

class WeightManager {
private:
  HX711 scale1;  // Celda VIDRIO
  HX711 scale2;  // Celda PLASTICO
  HX711 scale3;  // Celda METAL
  
  float lastWeight1;
  float lastWeight2; 
  float lastWeight3;
  
  unsigned long lastReading;
  bool isCalibrated1;
  bool isCalibrated2;
  bool isCalibrated3;
  
  // Filtro de promedio móvil
  float weightBuffer1[WEIGHT_SAMPLES];
  float weightBuffer2[WEIGHT_SAMPLES];
  float weightBuffer3[WEIGHT_SAMPLES];
  int bufferIndex;
  
  float calculateAverage(float buffer[]);
  bool isWeightStable(float currentWeight, float lastWeight);
  
public:
  WeightManager();
  bool begin();
  
  // Calibración
  void calibrateScale1(float knownWeight);
  void calibrateScale2(float knownWeight);
  void calibrateScale3(float knownWeight);
  void tareAll();
  void tareScale1();
  void tareScale2();
  void tareScale3();
  
  // Lectura de peso
  float readWeight1(bool filtered = true);
  float readWeight2(bool filtered = true);
  float readWeight3(bool filtered = true);
  void readAllWeights(float &weight1, float &weight2, float &weight3);
  
  // Validación por material
  bool isValidVidrioWeight(float weight);
  bool isValidPlasticoWeight(float weight);
  bool isValidMetalWeight(float weight);
  
  // Detección de objetos
  bool hasObjectOnScale1();
  bool hasObjectOnScale2();
  bool hasObjectOnScale3();
  bool hasAnyObject();
  
  // Estado de las celdas
  bool isScale1Ready();
  bool isScale2Ready();
  bool isScale3Ready();
  bool areAllScalesReady();
  
  // Información de diagnóstico
  void printWeights();
  void printCalibrationInfo();
  void printStatus();
  
  // Control avanzado
  void setScale1Factor(float factor);
  void setScale2Factor(float factor);
  void setScale3Factor(float factor);
  void powerDown();
  void powerUp();
  
  // Obtener material más probable según peso
  String predictMaterial(float weight);
  int getMaterialByWeight(float weight1, float weight2, float weight3);
};

#endif