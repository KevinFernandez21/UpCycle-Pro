#ifndef SIMPLE_CLASSIFIER_H
#define SIMPLE_CLASSIFIER_H

#include <Arduino.h>
#include "ConveyorMotor.h"
#include "PIRManager.h"
#include "ServoManager.h"
#include "OLEDDisplay.h"  // ← Agregamos OLED
#include "WeightManager.h" // ← Agregamos sistema de peso

// Estados del sistema
enum SystemState {
  IDLE,                    // Sistema inactivo
  BELT_RUNNING,           // Banda corriendo continuamente
  CAMERA_DETECTING,       // ESP32 CAM detectando objeto
  WAITING_FOR_PIR,        // Esperando confirmación PIR
  WAITING,                // Estado de espera
  PROCESSING              // Servo activado, procesando
};

class SimpleClassifier {
private:
  ConveyorMotor motor;
  PIRManager pirSensors;
  ServoManager servos;
  OLEDDisplay oledDisplay;  // ← Agregamos OLED
  WeightManager weightSensors; // ← Agregamos sistema de peso
  
  SystemState currentState;
  int selectedMode;  // 1=Vidrio, 2=Plástico, 3=Metal, 0=Ninguno
  unsigned long lastActivity;
  unsigned long timeout = 30000;  // 30 segundos timeout
  
  // Variables para logging de peso diario
  bool weightLoggingEnabled;
  float dailyWeightVidrio;
  float dailyWeightPlastico; 
  float dailyWeightMetal;
  unsigned long lastWeightReset;
  
  // Variables para ESP32 CAM + API
  String apiPrediction;           // Predicción de la API
  bool waitingForConfirmation;    // Esperando confirmación PIR
  unsigned long predictionTime;   // Timestamp de predicción
  unsigned long confirmationTimeout;
  
public:
  SimpleClassifier();
  bool begin();
  
  // Control principal
  void startContinuousOperation();  // Iniciar banda continua
  void stopSystem();
  void update();  // Llamar en loop()
  
  // Control ESP32 CAM + API
  void processApiPrediction(String prediction);  // Procesar predicción de API
  bool isWaitingForConfirmation() { return waitingForConfirmation; }
  String getCurrentPrediction() { return apiPrediction; }
  
  // Control de peso diario
  void enableDailyWeightLogging(bool enable = true);
  void resetDailyWeights();
  void addDailyWeight(String material, float weight);
  void printDailyWeights();
  
  // Estado
  SystemState getState() { return currentState; }
  int getMode() { return selectedMode; }
  void setMode(String material);
  
  // Métodos para ESP32 CAM + PIR
  bool checkPIRForPrediction();
  void processConfirmedDetection();
  float getDailyWeight(String material);
  
  // Utilidades
  void printStatus();
  void printHelp();
};

#endif