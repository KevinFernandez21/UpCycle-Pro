class ConfigService {
  static const String apiBaseUrl = 'http://192.168.137.102:8000';
  static const String esp32BaseUrl = 'http://192.168.0.109';
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
    'microcontrollerStatus': '/microcontroller/status',
    'sensorData': '/microcontroller/sensor/data',
    'motorControl': '/microcontroller/motor/control',
    'systemActivate': '/microcontroller/system/activate',
    'systemDeactivate': '/microcontroller/system/deactivate',
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