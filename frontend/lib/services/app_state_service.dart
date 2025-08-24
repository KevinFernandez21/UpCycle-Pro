import 'dart:async';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';

/// Servicio de estado global para la aplicación
class AppStateService extends ChangeNotifier {
  static final AppStateService _instance = AppStateService._internal();
  factory AppStateService() => _instance;
  AppStateService._internal();

  // Estado del material objetivo
  String _targetMaterial = 'PLÁSTICO';
  String get targetMaterial => _targetMaterial;

  // Estado de la última detección
  String? _lastDetectedMaterial;
  String? get lastDetectedMaterial => _lastDetectedMaterial;

  // Estado de confianza de la última detección
  String _lastConfidence = '0.0%';
  String get lastConfidence => _lastConfidence;

  // Estado de conectividad
  bool _isSystemConnected = false;
  bool get isSystemConnected => _isSystemConnected;

  // Estado de los sensores
  Map<String, bool> _sensorStates = {
    'PIR-1': false,
    'PIR-2': false,
    'PIR-3': false,
  };
  Map<String, bool> get sensorStates => Map.unmodifiable(_sensorStates);

  // Estado del motor
  bool _motorActive = false;
  bool get motorActive => _motorActive;

  // Controladores de stream para notificaciones en tiempo real
  final StreamController<String> _materialChangeController = StreamController<String>.broadcast();
  final StreamController<Map<String, dynamic>> _detectionController = StreamController<Map<String, dynamic>>.broadcast();

  Stream<String> get materialChangeStream => _materialChangeController.stream;
  Stream<Map<String, dynamic>> get detectionStream => _detectionController.stream;

  /// Actualiza el material objetivo
  void setTargetMaterial(String material) {
    if (_targetMaterial != material) {
      _targetMaterial = material;
      
      // Notificar a todos los listeners
      notifyListeners();
      
      // Enviar evento al stream
      _materialChangeController.add(material);
      
      debugPrint('Material objetivo cambiado a: $material');
    }
  }

  /// Actualiza el resultado de la última detección
  void updateDetectionResult({
    required String material,
    required String confidence,
  }) {
    bool changed = false;
    
    if (_lastDetectedMaterial != material) {
      _lastDetectedMaterial = material;
      changed = true;
    }
    
    if (_lastConfidence != confidence) {
      _lastConfidence = confidence;
      changed = true;
    }
    
    if (changed) {
      notifyListeners();
      
      // Enviar evento de detección
      _detectionController.add({
        'material': material,
        'confidence': confidence,
        'timestamp': DateTime.now(),
        'matches_target': material.toUpperCase() == _targetMaterial.toUpperCase(),
      });
      
      debugPrint('Detección actualizada: $material ($confidence)');
    }
  }

  /// Actualiza el estado de conectividad del sistema
  void updateSystemConnectivity(bool isConnected) {
    if (_isSystemConnected != isConnected) {
      _isSystemConnected = isConnected;
      notifyListeners();
      debugPrint('Estado de conectividad: ${isConnected ? "Conectado" : "Desconectado"}');
    }
  }

  /// Actualiza el estado de un sensor PIR específico
  void updateSensorState(String sensorId, bool isActive) {
    if (_sensorStates[sensorId] != isActive) {
      _sensorStates[sensorId] = isActive;
      notifyListeners();
      debugPrint('Sensor $sensorId: ${isActive ? "Activo" : "Inactivo"}');
    }
  }

  /// Actualiza el estado del motor
  void updateMotorState(bool isActive) {
    if (_motorActive != isActive) {
      _motorActive = isActive;
      notifyListeners();
      debugPrint('Motor: ${isActive ? "Activo" : "Inactivo"}');
    }
  }

  /// Actualiza múltiples estados de sensores PIR
  void updateAllSensorStates(Map<String, bool> states) {
    bool changed = false;
    
    states.forEach((sensorId, isActive) {
      if (_sensorStates[sensorId] != isActive) {
        _sensorStates[sensorId] = isActive;
        changed = true;
      }
    });
    
    if (changed) {
      notifyListeners();
      debugPrint('Estados de sensores actualizados: $states');
    }
  }

  /// Obtiene el color asociado con un material
  Color getMaterialColor(String material) {
    switch (material.toUpperCase()) {
      case 'PLÁSTICO':
        return const Color(0xFF4ECDC4);
      case 'VIDRIO':
        return const Color(0xFFFF6B6B);
      case 'METAL':
        return const Color(0xFFFECA57);
      default:
        return const Color(0xFF00aaff);
    }
  }

  /// Verifica si el material detectado coincide con el objetivo
  bool doesMaterialMatch(String detectedMaterial) {
    return detectedMaterial.toUpperCase() == _targetMaterial.toUpperCase();
  }

  /// Obtiene estadísticas de materiales (simuladas por ahora)
  Map<String, double> get materialStatistics {
    // En una implementación real, esto vendría del backend
    return {
      'PLÁSTICO': 45.0,
      'VIDRIO': 30.0,
      'METAL': 25.0,
    };
  }

  /// Reinicia todos los estados a valores por defecto
  void resetStates() {
    _targetMaterial = 'PLÁSTICO';
    _lastDetectedMaterial = null;
    _lastConfidence = '0.0%';
    _isSystemConnected = false;
    _sensorStates = {
      'PIR-1': false,
      'PIR-2': false,
      'PIR-3': false,
    };
    _motorActive = false;
    
    notifyListeners();
    debugPrint('Estados reiniciados');
  }

  @override
  void dispose() {
    _materialChangeController.close();
    _detectionController.close();
    super.dispose();
  }
}

/// Mixin para widgets que necesitan acceso al estado global
mixin AppStateMixin<T extends StatefulWidget> on State<T> {
  AppStateService get appState => AppStateService();

  @override
  void initState() {
    super.initState();
    appState.addListener(_onAppStateChanged);
  }

  @override
  void dispose() {
    appState.removeListener(_onAppStateChanged);
    super.dispose();
  }

  void _onAppStateChanged() {
    if (mounted) {
      setState(() {
        // Forzar rebuild cuando cambie el estado global
      });
    }
  }
}

/// Widget builder que se reconstruye cuando cambia el estado global
class AppStateBuilder extends StatefulWidget {
  final Widget Function(BuildContext context, AppStateService appState) builder;

  const AppStateBuilder({
    super.key,
    required this.builder,
  });

  @override
  State<AppStateBuilder> createState() => _AppStateBuilderState();
}

class _AppStateBuilderState extends State<AppStateBuilder> with AppStateMixin {
  @override
  Widget build(BuildContext context) {
    return widget.builder(context, appState);
  }
}