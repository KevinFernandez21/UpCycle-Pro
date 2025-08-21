import 'dart:convert';
import 'dart:io';
import 'package:http/http.dart' as http;
import 'package:flutter/foundation.dart';
import 'config_service.dart';

class ApiService {
  static final String _baseUrl = ConfigService.apiBaseUrl;
  static const Duration _timeout = ConfigService.apiTimeout;
  
  static final ApiService _instance = ApiService._internal();
  factory ApiService() => _instance;
  ApiService._internal();

  Future<Map<String, dynamic>> get(String endpoint) async {
    try {
      final response = await http
          .get(
            Uri.parse('$_baseUrl$endpoint'),
            headers: {'Content-Type': 'application/json'},
          )
          .timeout(_timeout);
      
      return _handleResponse(response);
    } catch (e) {
      throw ApiException('Error en petición GET: $e');
    }
  }

  Future<Map<String, dynamic>> post(
      String endpoint, Map<String, dynamic> data) async {
    try {
      final response = await http
          .post(
            Uri.parse('$_baseUrl$endpoint'),
            headers: {'Content-Type': 'application/json'},
            body: json.encode(data),
          )
          .timeout(_timeout);
      
      return _handleResponse(response);
    } catch (e) {
      throw ApiException('Error en petición POST: $e');
    }
  }

  Future<Map<String, dynamic>> uploadImage(String imagePath) async {
    try {
      var request = http.MultipartRequest(
        'POST',
        Uri.parse('$_baseUrl/predict'),
      );

      var file = await http.MultipartFile.fromPath('file', imagePath);
      request.files.add(file);

      var streamedResponse = await request.send().timeout(_timeout);
      var response = await http.Response.fromStream(streamedResponse);
      
      return _handleResponse(response);
    } catch (e) {
      throw ApiException('Error subiendo imagen: $e');
    }
  }

  Future<List<Map<String, dynamic>>> uploadMultipleImages(
      List<String> imagePaths) async {
    try {
      var request = http.MultipartRequest(
        'POST',
        Uri.parse('$_baseUrl/predict_batch'),
      );

      for (String imagePath in imagePaths) {
        var file = await http.MultipartFile.fromPath('files', imagePath);
        request.files.add(file);
      }

      var streamedResponse = await request.send().timeout(_timeout);
      var response = await http.Response.fromStream(streamedResponse);
      
      final result = _handleResponse(response);
      return List<Map<String, dynamic>>.from(result['predictions'] ?? []);
    } catch (e) {
      throw ApiException('Error subiendo múltiples imágenes: $e');
    }
  }

  Future<Map<String, dynamic>> getSystemStatus() async {
    return await get('/health');
  }

  Future<Map<String, dynamic>> getModelInfo() async {
    return await get('/model_info');
  }

  Future<Map<String, dynamic>> getApiInfo() async {
    return await get('/');
  }

  Future<Map<String, dynamic>> getAvailableModels() async {
    return await get('/available_models');
  }

  Future<Map<String, dynamic>> loadModel({String? modelName}) async {
    final endpoint = modelName != null 
        ? '/load_model?model_name=$modelName'
        : '/load_model';
    return await post(endpoint, {});
  }

  Map<String, dynamic> _handleResponse(http.Response response) {
    if (response.statusCode >= 200 && response.statusCode < 300) {
      try {
        return json.decode(response.body);
      } catch (e) {
        throw ApiException('Error decodificando respuesta JSON');
      }
    } else {
      String errorMessage;
      try {
        final errorData = json.decode(response.body);
        errorMessage = errorData['detail'] ?? 'Error desconocido';
      } catch (e) {
        errorMessage = 'Error HTTP ${response.statusCode}';
      }
      throw ApiException(errorMessage);
    }
  }
}

class ApiException implements Exception {
  final String message;
  ApiException(this.message);
  
  @override
  String toString() => 'ApiException: $message';
}

class PredictionResult {
  final String predictedClass;
  final double confidence;
  final Map<String, double> allProbabilities;
  final DateTime timestamp;
  final String? filename;

  PredictionResult({
    required this.predictedClass,
    required this.confidence,
    required this.allProbabilities,
    required this.timestamp,
    this.filename,
  });

  factory PredictionResult.fromJson(Map<String, dynamic> json) {
    return PredictionResult(
      predictedClass: json['predicted_class'],
      confidence: json['confidence'].toDouble(),
      allProbabilities: Map<String, double>.from(
        json['all_probabilities'].map((k, v) => MapEntry(k, v.toDouble())),
      ),
      timestamp: DateTime.parse(json['timestamp']),
      filename: json['image_info']?['filename'],
    );
  }

  String get materialType {
    switch (predictedClass.toLowerCase()) {
      case 'glass':
        return 'Vidrio';
      case 'plastic':
        return 'Plástico';
      case 'metal':
        return 'Metal';
      default:
        return predictedClass;
    }
  }

  String get confidencePercentage => '${(confidence * 100).toStringAsFixed(1)}%';
}