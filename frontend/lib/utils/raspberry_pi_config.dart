/// Configuraciones optimizadas para Raspberry Pi 5
class RaspberryPiConfig {
  // Configuraciones de rendimiento
  static const int maxConcurrentRequests = 3;
  static const Duration networkTimeout = Duration(seconds: 10);
  static const Duration cacheTimeout = Duration(minutes: 5);
  
  // Configuraciones de UI optimizadas para ARM64
  static const Duration animationDuration = Duration(milliseconds: 300);
  static const Duration pageTransitionDuration = Duration(milliseconds: 250);
  static const int maxFrameRate = 30; // Limitar FPS para ahorrar CPU
  
  // Configuraciones de memoria
  static const int maxCachedImages = 5;
  static const int maxImageSizeBytes = 2 * 1024 * 1024; // 2MB
  
  // Configuraciones de red optimizadas
  static const Duration systemUpdateInterval = Duration(seconds: 8);
  static const Duration sensorUpdateInterval = Duration(seconds: 6);
  static const Duration cameraStreamInterval = Duration(milliseconds: 500);
  
  // Configuraciones de CPU
  static const bool enableGpuAcceleration = true;
  static const bool enableImageOptimization = true;
  static const bool enableLazyLoading = true;
  
  // Configuraciones específicas para ESP32
  static const Duration esp32ConnectionTimeout = Duration(seconds: 5);
  static const int esp32RetryAttempts = 3;
  static const Duration esp32RetryDelay = Duration(seconds: 2);
  
  /// Verifica si estamos ejecutando en Raspberry Pi
  static bool get isRaspberryPi {
    // En producción, esto detectaría el hardware real
    // Por ahora, asumimos que es RPi si es Linux ARM64
    return true; // Placeholder
  }
  
  /// Configuraciones de calidad de imagen basadas en el rendimiento
  static Map<String, dynamic> get imageQualitySettings {
    return {
      'compressionQuality': 0.7, // Reducir calidad para mejor rendimiento
      'maxWidth': 800,
      'maxHeight': 600,
      'enableCaching': true,
      'filterQuality': 'medium', // Usar filtrado medio en lugar de alto
    };
  }
  
  /// Configuraciones de animaciones optimizadas
  static Map<String, dynamic> get animationSettings {
    return {
      'enableComplexAnimations': false,
      'enableParticleEffects': false,
      'enableShaderEffects': false,
      'maxSimultaneousAnimations': 3,
      'preferCpuRendering': true, // Usar CPU en lugar de GPU para animaciones simples
    };
  }
  
  /// Configuraciones de red optimizadas para conexiones locales
  static Map<String, dynamic> get networkSettings {
    return {
      'connectionPooling': true,
      'keepAliveTimeout': Duration(seconds: 30),
      'maxConnectionsPerHost': 2,
      'enableGzipCompression': false, // Ahorrar CPU en redes locales rápidas
      'enableCertificateValidation': false, // Para desarrollo local
    };
  }
  
  /// Configuraciones de base de datos/cache local
  static Map<String, dynamic> get storageSettings {
    return {
      'enableSqliteWal': true, // Write-Ahead Logging para mejor rendimiento
      'cacheSize': 10 * 1024 * 1024, // 10MB cache
      'enableCompression': false, // Ahorrar CPU
      'maxDatabaseSize': 100 * 1024 * 1024, // 100MB máximo
    };
  }
  
  /// Configuraciones específicas para el sistema de clasificación
  static Map<String, dynamic> get classificationSettings {
    return {
      'enableBatchProcessing': true,
      'maxBatchSize': 5,
      'enablePreprocessing': false, // Dejar el preprocesamiento al backend
      'enableLocalCaching': true,
      'maxCacheEntries': 50,
    };
  }
}

/// Utilidades de monitoreo de rendimiento para Raspberry Pi
class PerformanceMonitor {
  static DateTime _lastFrameTime = DateTime.now();
  static List<double> _frameTimes = [];
  static const int _maxFrameTimesSamples = 60;
  
  /// Registra el tiempo de frame para monitoreo de FPS
  static void recordFrameTime() {
    final now = DateTime.now();
    final frameTime = now.difference(_lastFrameTime).inMilliseconds.toDouble();
    _lastFrameTime = now;
    
    _frameTimes.add(frameTime);
    if (_frameTimes.length > _maxFrameTimesSamples) {
      _frameTimes.removeAt(0);
    }
  }
  
  /// Obtiene los FPS promedio
  static double get averageFps {
    if (_frameTimes.isEmpty) return 0.0;
    final averageFrameTime = _frameTimes.reduce((a, b) => a + b) / _frameTimes.length;
    return averageFrameTime > 0 ? 1000.0 / averageFrameTime : 0.0;
  }
  
  /// Detecta si el rendimiento está degradado
  static bool get isPerformanceDegraded {
    return averageFps < 15.0; // Menos de 15 FPS indica problemas
  }
  
  /// Obtiene recomendaciones de optimización
  static List<String> get optimizationRecommendations {
    final recommendations = <String>[];
    
    if (isPerformanceDegraded) {
      recommendations.add('Reducir frecuencia de actualizaciones');
      recommendations.add('Desactivar animaciones complejas');
      recommendations.add('Reducir calidad de imagen');
    }
    
    if (averageFps < 10.0) {
      recommendations.add('Cerrar aplicaciones innecesarias');
      recommendations.add('Verificar temperatura del sistema');
      recommendations.add('Considerar reinicio del sistema');
    }
    
    return recommendations;
  }
}