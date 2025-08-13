import 'dart:convert';
import 'package:http/http.dart' as http;
import 'api_service.dart';
import 'config_service.dart';

class MicrocontrollerService {
  static final MicrocontrollerService _instance = MicrocontrollerService._internal();
  factory MicrocontrollerService() => _instance;
  MicrocontrollerService._internal();

  final ApiService _apiService = ApiService();

  Future<SystemStatus> getSystemStatus() async {
    try {
      final response = await _apiService.get('/microcontroller/status');
      return SystemStatus.fromJson(response);
    } catch (e) {
      throw ApiException('Error obteniendo estado del sistema: $e');
    }
  }

  Future<SensorData> getSensorData() async {
    try {
      final response = await _apiService.get('/microcontroller/sensor/data');
      return SensorData.fromJson(response);
    } catch (e) {
      throw ApiException('Error obteniendo datos de sensores: $e');
    }
  }

  Future<bool> activateSystem() async {
    try {
      final response = await _apiService.post('/microcontroller/system/activate', {});
      return response['status'] == 'success';
    } catch (e) {
      throw ApiException('Error activando sistema: $e');
    }
  }

  Future<bool> deactivateSystem() async {
    try {
      final response = await _apiService.post('/microcontroller/system/deactivate', {});
      return response['status'] == 'success';
    } catch (e) {
      throw ApiException('Error desactivando sistema: $e');
    }
  }

  Future<bool> controlMotor({
    required bool conveyorActive,
    required int servoPosition,
    int? duration,
  }) async {
    try {
      final response = await _apiService.post('/microcontroller/motor/control', {
        'conveyor_active': conveyorActive,
        'servo_position': servoPosition,
        if (duration != null) 'duration': duration,
      });
      return response['status'] == 'success';
    } catch (e) {
      throw ApiException('Error controlando motor: $e');
    }
  }

  Future<bool> separateMaterial(String materialType) async {
    try {
      final servoPosition = ConfigService.getServoPosition(materialType);
      return await controlMotor(
        conveyorActive: true,
        servoPosition: servoPosition,
        duration: 5, // 5 segundos de operación
      );
    } catch (e) {
      throw ApiException('Error separando material: $e');
    }
  }
}

class SystemStatus {
  final bool systemActive;
  final String? lastClassification;
  final String? motorStatus;
  final int? servoPosition;

  SystemStatus({
    required this.systemActive,
    this.lastClassification,
    this.motorStatus,
    this.servoPosition,
  });

  factory SystemStatus.fromJson(Map<String, dynamic> json) {
    return SystemStatus(
      systemActive: json['system_active'] ?? false,
      lastClassification: json['last_classification'],
      motorStatus: json['motor_status'],
      servoPosition: json['servo_position'],
    );
  }
}

class SensorData {
  final bool pirSensor;
  final double weight;
  final String? lastUpdate;

  SensorData({
    required this.pirSensor,
    required this.weight,
    this.lastUpdate,
  });

  factory SensorData.fromJson(Map<String, dynamic> json) {
    return SensorData(
      pirSensor: json['pir_sensor'] ?? false,
      weight: (json['weight'] ?? 0.0).toDouble(),
      lastUpdate: json['last_update'],
    );
  }

  String get weightDisplay => '${weight.toStringAsFixed(2)} kg';
  String get statusDisplay => pirSensor ? 'Objeto Detectado' : 'Sin Objeto';
}