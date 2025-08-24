import 'dart:convert';
import 'dart:typed_data';
import 'package:http/http.dart' as http;
import 'package:flutter/foundation.dart';
import 'api_service.dart';
import 'config_service.dart';

class SystemService {
  static final SystemService _instance = SystemService._internal();
  factory SystemService() => _instance;
  SystemService._internal();

  final ApiService _apiService = ApiService();

  // System Status
  Future<SystemStatus> getSystemStatus() async {
    try {
      final response = await _apiService.get('/system/status');
      return SystemStatus.fromJson(response);
    } catch (e) {
      throw ApiException('Error obteniendo estado del sistema: $e');
    }
  }

  Future<SystemInfo> getSystemInfo() async {
    try {
      final response = await _apiService.get('/system/info');
      return SystemInfo.fromJson(response);
    } catch (e) {
      throw ApiException('Error obteniendo información del sistema: $e');
    }
  }

  Future<SystemMetrics> getSystemMetrics() async {
    try {
      final response = await _apiService.get('/system/metrics');
      return SystemMetrics.fromJson(response);
    } catch (e) {
      throw ApiException('Error obteniendo métricas del sistema: $e');
    }
  }

  // ESP32-CAM Control
  Future<Esp32CamStatus> getEsp32CamStatus() async {
    try {
      final response = await _apiService.get('/esp32-cam/status');
      return Esp32CamStatus.fromJson(response);
    } catch (e) {
      throw ApiException('Error obteniendo estado ESP32-CAM: $e');
    }
  }

  Future<Uint8List?> captureImageFromEsp32() async {
    try {
      final response = await _apiService.get('/esp32-cam/capture');
      if (response['image_data'] != null) {
        return base64Decode(response['image_data']);
      }
      return null;
    } catch (e) {
      throw ApiException('Error capturando imagen ESP32-CAM: $e');
    }
  }

  Future<bool> controlEsp32Cam({
    bool? flashEnabled,
    int? quality,
    String? action,
  }) async {
    try {
      final data = <String, dynamic>{};
      if (flashEnabled != null) data['flash_enabled'] = flashEnabled;
      if (quality != null) data['quality'] = quality;
      if (action != null) data['action'] = action;

      final response = await _apiService.post('/esp32-cam/control', data);
      return response['success'] == true;
    } catch (e) {
      throw ApiException('Error controlando ESP32-CAM: $e');
    }
  }

  // ESP32-CONTROL
  Future<Esp32ControlStatus> getEsp32ControlStatus() async {
    try {
      final response = await _apiService.get('/esp32-control/status');
      return Esp32ControlStatus.fromJson(response);
    } catch (e) {
      throw ApiException('Error obteniendo estado ESP32-CONTROL: $e');
    }
  }

  Future<List<PirSensorData>> getAllPirSensors() async {
    try {
      final response = await _apiService.get('/esp32-control/sensors/pir');
      return (response['sensors'] as List)
          .map((sensor) => PirSensorData.fromJson(sensor))
          .toList();
    } catch (e) {
      throw ApiException('Error obteniendo sensores PIR: $e');
    }
  }

  Future<List<WeightSensorData>> getAllWeightSensors() async {
    try {
      final response = await _apiService.get('/esp32-control/sensors/weight');
      return (response['sensors'] as List)
          .map((sensor) => WeightSensorData.fromJson(sensor))
          .toList();
    } catch (e) {
      throw ApiException('Error obteniendo sensores de peso: $e');
    }
  }

  // Sensor Data
  Future<SensorData> getSensorData() async {
    try {
      final response = await _apiService.get('/sensors/data');
      return SensorData.fromJson(response);
    } catch (e) {
      throw ApiException('Error obteniendo datos de sensores: $e');
    }
  }

  Future<List<SensorReading>> getSensorHistory({
    DateTime? from,
    DateTime? to,
    int? limit,
  }) async {
    try {
      final params = <String, String>{};
      if (from != null) params['from'] = from.toIso8601String();
      if (to != null) params['to'] = to.toIso8601String();
      if (limit != null) params['limit'] = limit.toString();

      String endpoint = '/sensors/history';
      if (params.isNotEmpty) {
        final query = params.entries.map((e) => '${e.key}=${e.value}').join('&');
        endpoint += '?$query';
      }

      final response = await _apiService.get(endpoint);
      return (response['readings'] as List)
          .map((reading) => SensorReading.fromJson(reading))
          .toList();
    } catch (e) {
      throw ApiException('Error obteniendo historial de sensores: $e');
    }
  }

  // Motor Control
  Future<bool> controlMotor({
    bool? conveyorActive,
    int? speed,
    int? duration,
  }) async {
    try {
      final data = <String, dynamic>{};
      if (conveyorActive != null) data['conveyor_active'] = conveyorActive;
      if (speed != null) data['speed'] = speed;
      if (duration != null) data['duration'] = duration;

      final response = await _apiService.post('/motor/control', data);
      return response['success'] == true;
    } catch (e) {
      throw ApiException('Error controlando motor: $e');
    }
  }

  Future<bool> controlServo({
    required int position,
    int? speed,
  }) async {
    try {
      final data = {
        'position': position,
        if (speed != null) 'speed': speed,
      };

      final response = await _apiService.post('/servo/control', data);
      return response['success'] == true;
    } catch (e) {
      throw ApiException('Error controlando servo: $e');
    }
  }

  Future<MotorStatus> getMotorStatus() async {
    try {
      final response = await _apiService.get('/motor/status');
      return MotorStatus.fromJson(response);
    } catch (e) {
      throw ApiException('Error obteniendo estado del motor: $e');
    }
  }

  // Classification
  Future<PredictionResult?> classifyImage(Uint8List imageBytes) async {
    try {
      // Convertir a base64 para enviar al backend
      final base64Image = base64Encode(imageBytes);
      final response = await _apiService.post('/classify/image', {
        'image_data': base64Image,
      });
      return PredictionResult.fromJson(response);
    } catch (e) {
      throw ApiException('Error clasificando imagen: $e');
    }
  }

  Future<List<ClassificationRecord>> getClassificationHistory({
    DateTime? from,
    DateTime? to,
    int? limit,
  }) async {
    try {
      final params = <String, String>{};
      if (from != null) params['from'] = from.toIso8601String();
      if (to != null) params['to'] = to.toIso8601String();
      if (limit != null) params['limit'] = limit.toString();

      String endpoint = '/classify/history';
      if (params.isNotEmpty) {
        final query = params.entries.map((e) => '${e.key}=${e.value}').join('&');
        endpoint += '?$query';
      }

      final response = await _apiService.get(endpoint);
      return (response['classifications'] as List)
          .map((record) => ClassificationRecord.fromJson(record))
          .toList();
    } catch (e) {
      throw ApiException('Error obteniendo historial de clasificaciones: $e');
    }
  }

  // Automated Material Separation
  Future<bool> separateMaterial(String materialType) async {
    try {
      final servoPosition = ConfigService.getServoPosition(materialType);
      
      // Activar cinta transportadora y mover servo
      final motorSuccess = await controlMotor(
        conveyorActive: true,
        duration: 5000, // 5 segundos
      );
      
      if (motorSuccess) {
        final servoSuccess = await controlServo(position: servoPosition);
        return servoSuccess;
      }
      
      return false;
    } catch (e) {
      throw ApiException('Error separando material: $e');
    }
  }
}

// Models
class SystemStatus {
  final bool isActive;
  final String status;
  final DateTime lastUpdate;
  final Map<String, dynamic> components;

  SystemStatus({
    required this.isActive,
    required this.status,
    required this.lastUpdate,
    required this.components,
  });

  factory SystemStatus.fromJson(Map<String, dynamic> json) {
    return SystemStatus(
      isActive: json['is_active'] ?? false,
      status: json['status'] ?? 'Unknown',
      lastUpdate: DateTime.parse(json['last_update']),
      components: Map<String, dynamic>.from(json['components'] ?? {}),
    );
  }
}

class SystemInfo {
  final String version;
  final String environment;
  final Map<String, dynamic> hardware;
  final Map<String, dynamic> software;

  SystemInfo({
    required this.version,
    required this.environment,
    required this.hardware,
    required this.software,
  });

  factory SystemInfo.fromJson(Map<String, dynamic> json) {
    return SystemInfo(
      version: json['version'] ?? '1.0.0',
      environment: json['environment'] ?? 'production',
      hardware: Map<String, dynamic>.from(json['hardware'] ?? {}),
      software: Map<String, dynamic>.from(json['software'] ?? {}),
    );
  }
}

class SystemMetrics {
  final double cpuUsage;
  final double memoryUsage;
  final double diskUsage;
  final Map<String, double> sensors;

  SystemMetrics({
    required this.cpuUsage,
    required this.memoryUsage,
    required this.diskUsage,
    required this.sensors,
  });

  factory SystemMetrics.fromJson(Map<String, dynamic> json) {
    return SystemMetrics(
      cpuUsage: (json['cpu_usage'] ?? 0.0).toDouble(),
      memoryUsage: (json['memory_usage'] ?? 0.0).toDouble(),
      diskUsage: (json['disk_usage'] ?? 0.0).toDouble(),
      sensors: Map<String, double>.from(
        (json['sensors'] ?? {}).map((k, v) => MapEntry(k, v.toDouble())),
      ),
    );
  }
}

class Esp32CamStatus {
  final bool isConnected;
  final String firmwareVersion;
  final bool cameraReady;
  final Map<String, dynamic> settings;

  Esp32CamStatus({
    required this.isConnected,
    required this.firmwareVersion,
    required this.cameraReady,
    required this.settings,
  });

  factory Esp32CamStatus.fromJson(Map<String, dynamic> json) {
    return Esp32CamStatus(
      isConnected: json['is_connected'] ?? false,
      firmwareVersion: json['firmware_version'] ?? 'Unknown',
      cameraReady: json['camera_ready'] ?? false,
      settings: Map<String, dynamic>.from(json['settings'] ?? {}),
    );
  }
}

class Esp32ControlStatus {
  final bool isConnected;
  final String firmwareVersion;
  final bool sensorsReady;
  final bool motorReady;
  final Map<String, dynamic> settings;

  Esp32ControlStatus({
    required this.isConnected,
    required this.firmwareVersion,
    required this.sensorsReady,
    required this.motorReady,
    required this.settings,
  });

  factory Esp32ControlStatus.fromJson(Map<String, dynamic> json) {
    return Esp32ControlStatus(
      isConnected: json['is_connected'] ?? false,
      firmwareVersion: json['firmware_version'] ?? 'Unknown',
      sensorsReady: json['sensors_ready'] ?? false,
      motorReady: json['motor_ready'] ?? false,
      settings: Map<String, dynamic>.from(json['settings'] ?? {}),
    );
  }
}

class PirSensorData {
  final String id;
  final bool isActive;
  final double value;
  final DateTime lastUpdate;

  PirSensorData({
    required this.id,
    required this.isActive,
    required this.value,
    required this.lastUpdate,
  });

  factory PirSensorData.fromJson(Map<String, dynamic> json) {
    return PirSensorData(
      id: json['id'],
      isActive: json['is_active'] ?? false,
      value: (json['value'] ?? 0.0).toDouble(),
      lastUpdate: DateTime.parse(json['last_update']),
    );
  }
}

class WeightSensorData {
  final String id;
  final double weight;
  final bool isCalibrated;
  final DateTime lastUpdate;

  WeightSensorData({
    required this.id,
    required this.weight,
    required this.isCalibrated,
    required this.lastUpdate,
  });

  factory WeightSensorData.fromJson(Map<String, dynamic> json) {
    return WeightSensorData(
      id: json['id'],
      weight: (json['weight'] ?? 0.0).toDouble(),
      isCalibrated: json['is_calibrated'] ?? false,
      lastUpdate: DateTime.parse(json['last_update']),
    );
  }

  String get weightDisplay => '${weight.toStringAsFixed(2)} kg';
}

class SensorData {
  final bool pirSensor;
  final double weight;
  final DateTime timestamp;
  final Map<String, dynamic> additionalSensors;

  SensorData({
    required this.pirSensor,
    required this.weight,
    required this.timestamp,
    required this.additionalSensors,
  });

  factory SensorData.fromJson(Map<String, dynamic> json) {
    return SensorData(
      pirSensor: json['pir_sensor'] ?? false,
      weight: (json['weight'] ?? 0.0).toDouble(),
      timestamp: DateTime.parse(json['timestamp']),
      additionalSensors: Map<String, dynamic>.from(json['additional_sensors'] ?? {}),
    );
  }

  String get weightDisplay => '${weight.toStringAsFixed(2)} kg';
  String get statusDisplay => pirSensor ? 'Objeto Detectado' : 'Sin Objeto';
}

class SensorReading {
  final String sensorType;
  final double value;
  final DateTime timestamp;

  SensorReading({
    required this.sensorType,
    required this.value,
    required this.timestamp,
  });

  factory SensorReading.fromJson(Map<String, dynamic> json) {
    return SensorReading(
      sensorType: json['sensor_type'],
      value: (json['value'] ?? 0.0).toDouble(),
      timestamp: DateTime.parse(json['timestamp']),
    );
  }
}

class MotorStatus {
  final bool isActive;
  final int currentSpeed;
  final int servoPosition;
  final String state;

  MotorStatus({
    required this.isActive,
    required this.currentSpeed,
    required this.servoPosition,
    required this.state,
  });

  factory MotorStatus.fromJson(Map<String, dynamic> json) {
    return MotorStatus(
      isActive: json['is_active'] ?? false,
      currentSpeed: json['current_speed'] ?? 0,
      servoPosition: json['servo_position'] ?? 90,
      state: json['state'] ?? 'idle',
    );
  }
}

class ClassificationRecord {
  final String materialType;
  final double confidence;
  final DateTime timestamp;
  final Map<String, double> probabilities;

  ClassificationRecord({
    required this.materialType,
    required this.confidence,
    required this.timestamp,
    required this.probabilities,
  });

  factory ClassificationRecord.fromJson(Map<String, dynamic> json) {
    return ClassificationRecord(
      materialType: json['material_type'],
      confidence: (json['confidence'] ?? 0.0).toDouble(),
      timestamp: DateTime.parse(json['timestamp']),
      probabilities: Map<String, double>.from(
        (json['probabilities'] ?? {}).map((k, v) => MapEntry(k, v.toDouble())),
      ),
    );
  }
}