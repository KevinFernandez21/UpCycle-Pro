class ConfigService {
  static const String apiBaseUrl = 'http://192.168.137.102:8000';
  static const String esp32BaseUrl = 'http://192.168.0.109';
  // Las IPs de ESP32 ahora se obtienen del backend
  static const Duration apiTimeout = Duration(seconds: 30);
  
  // CNN Model Configuration
  static const List<String> materialClasses = ['glass', 'metal', 'plastic'];
  static const double confidenceThreshold = 0.7;
  
  // Theme Configuration
  static const Map<String, dynamic> theme = {
    'primaryColor': 0xFF00aaff,
    'backgroundGradient': [
      0xFF1a1a2e,
      0xFF16213e,
      0xFF0f3460,
    ],
    'textColors': {
      'primary': 0xFFFFFFFF,
      'secondary': 0xFF00aaff,
      'accent': 0xFF4ecdc4,
    }
  };
  
  // Servo Positions for Material Separation
  static const Map<String, int> servoPositions = {
    'glass': 45,
    'plastic': 90,
    'metal': 135,
  };
  
  // Camera Configuration
  static const Map<String, dynamic> camera = {
    'defaultResolution': 'high',
    'imageQuality': 80,
    'enableAudio': false,
  };
  
  // ESP32 Camera Configuration
  static const Map<String, dynamic> esp32Camera = {
    'streamEndpoint': '/stream',
    'captureEndpoint': '/capture',
    'statusEndpoint': '/status',
    'qualityEndpoint': '/quality',
    'flashEndpoint': '/flash',
    'restartEndpoint': '/restart',
    'frameRate': 10, // FPS
    'defaultQuality': 12, // 0-63, higher is better quality
    'timeout': 10, // seconds
  };
  
  // Sensor Configuration
  static const Map<String, dynamic> sensors = {
    'weightThreshold': 0.1,  // kg
    'updateInterval': 1000,  // milliseconds
  };
  
  // App Information
  static const String appName = 'UpCycle Pro';
  static const String appVersion = '1.0.0';
  static const String appDescription = 'Sistema de clasificación automática de materiales reciclables';
  
  // API Endpoints  
  static const Map<String, String> endpoints = {
    'root': '/',
    'health': '/health',
    'predict': '/predict',
    'predictBatch': '/predict_batch',
    'modelInfo': '/model_info',
    'availableModels': '/available_models',
    'loadModel': '/load_model',
    
    // Sistema endpoints
    'systemStatus': '/system/status',
    'systemInfo': '/system/info',
    'systemMetrics': '/system/metrics',
    'esp32Config': '/system/esp32-config',
    
    // ESP32-CAM endpoints (via FastAPI)
    'esp32CamStatus': '/esp32-cam/status',
    'esp32CamCapture': '/esp32-cam/capture',
    'esp32CamControl': '/esp32-cam/control',
    'esp32CamSettings': '/esp32-cam/settings',
    
    // ESP32-CONTROL endpoints (via FastAPI)
    'esp32ControlStatus': '/esp32-control/status',
    'esp32ControlSensors': '/esp32-control/sensors',
    'esp32ControlMotors': '/esp32-control/motors',
    'esp32ControlServos': '/esp32-control/servos',
    
    // Sensor endpoints (via FastAPI)
    'sensorData': '/sensors/data',
    'sensorHistory': '/sensors/history',
    'sensorCalibrate': '/sensors/calibrate',
    
    // Motor control endpoints (via FastAPI) 
    'motorControl': '/motor/control',
    'motorStatus': '/motor/status',
    'servoControl': '/servo/control',
    'conveyorControl': '/conveyor/control',
    
    // Classification endpoints
    'classifyImage': '/classify/image',
    'classifyBatch': '/classify/batch',
    'classificationHistory': '/classify/history',
  };
  
  static String getFullUrl(String endpoint) {
    return '$apiBaseUrl${endpoints[endpoint] ?? endpoint}';
  }
  
  static String getMaterialDisplayName(String materialKey) {
    switch (materialKey.toLowerCase()) {
      case 'glass':
        return 'Vidrio';
      case 'plastic':
        return 'Plástico';
      case 'metal':
        return 'Metal';
      default:
        return materialKey;
    }
  }
  
  static int getServoPosition(String material) {
    return servoPositions[material.toLowerCase()] ?? 90;
  }
}