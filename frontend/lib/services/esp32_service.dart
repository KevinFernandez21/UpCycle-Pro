import 'dart:convert';
import 'dart:io';
import 'dart:typed_data';
import 'package:http/http.dart' as http;
import 'package:flutter/foundation.dart';
import 'package:path_provider/path_provider.dart';
import 'config_service.dart';
import 'api_service.dart';

class Esp32Service {
  static final Esp32Service _instance = Esp32Service._internal();
  factory Esp32Service() => _instance;
  Esp32Service._internal();

  String get _esp32BaseUrl => ConfigService.esp32BaseUrl;
  final Duration _timeout = const Duration(seconds: 10);

  Future<Uint8List?> captureImage() async {
    try {
      final response = await http
          .get(
            Uri.parse('$_esp32BaseUrl/capture'),
            headers: {'Content-Type': 'application/json'},
          )
          .timeout(_timeout);

      if (response.statusCode == 200) {
        return response.bodyBytes;
      } else {
        throw Exception('Error capturando imagen: ${response.statusCode}');
      }
    } catch (e) {
      debugPrint('Error conectando con ESP32: $e');
      return null;
    }
  }

  Future<Uint8List?> getStreamFrame() async {
    try {
      final response = await http
          .get(
            Uri.parse('$_esp32BaseUrl/stream'),
            headers: {'Content-Type': 'application/json'},
          )
          .timeout(_timeout);

      if (response.statusCode == 200) {
        return response.bodyBytes;
      } else {
        throw Exception('Error obteniendo frame: ${response.statusCode}');
      }
    } catch (e) {
      debugPrint('Error obteniendo stream: $e');
      return null;
    }
  }

  Future<bool> setFlashlight(bool enabled) async {
    try {
      final response = await http
          .post(
            Uri.parse('$_esp32BaseUrl/flash'),
            headers: {'Content-Type': 'application/json'},
            body: json.encode({'enabled': enabled}),
          )
          .timeout(_timeout);

      return response.statusCode == 200;
    } catch (e) {
      debugPrint('Error controlando flash: $e');
      return false;
    }
  }

  Future<bool> setImageQuality(int quality) async {
    try {
      final response = await http
          .post(
            Uri.parse('$_esp32BaseUrl/quality'),
            headers: {'Content-Type': 'application/json'},
            body: json.encode({'quality': quality}),
          )
          .timeout(_timeout);

      return response.statusCode == 200;
    } catch (e) {
      debugPrint('Error configurando calidad: $e');
      return false;
    }
  }

  Future<Map<String, dynamic>?> getStatus() async {
    try {
      final response = await http
          .get(
            Uri.parse('$_esp32BaseUrl/status'),
            headers: {'Content-Type': 'application/json'},
          )
          .timeout(_timeout);

      if (response.statusCode == 200) {
        return json.decode(response.body);
      } else {
        throw Exception('Error obteniendo estado: ${response.statusCode}');
      }
    } catch (e) {
      debugPrint('Error obteniendo estado ESP32: $e');
      return null;
    }
  }

  Future<bool> restartCamera() async {
    try {
      final response = await http
          .post(
            Uri.parse('$_esp32BaseUrl/restart'),
            headers: {'Content-Type': 'application/json'},
          )
          .timeout(_timeout);

      return response.statusCode == 200;
    } catch (e) {
      debugPrint('Error reiniciando cámara: $e');
      return false;
    }
  }

  Future<File?> saveImageBytes(Uint8List imageBytes) async {
    try {
      final appDir = await getApplicationDocumentsDirectory();
      final fileName = 'esp32_captured_${DateTime.now().millisecondsSinceEpoch}.jpg';
      final file = File('${appDir.path}/$fileName');
      await file.writeAsBytes(imageBytes);
      return file;
    } catch (e) {
      debugPrint('Error guardando imagen: $e');
      return null;
    }
  }

  Future<PredictionResult?> classifyImage(Uint8List imageBytes) async {
    try {
      final file = await saveImageBytes(imageBytes);
      if (file != null) {
        final apiService = ApiService();
        final result = await apiService.uploadImage(file.path);
        
        // Limpiar archivo temporal
        try {
          await file.delete();
        } catch (e) {
          debugPrint('Error eliminando archivo temporal: $e');
        }
        
        return PredictionResult.fromJson(result);
      }
      return null;
    } catch (e) {
      debugPrint('Error clasificando imagen ESP32: $e');
      return null;
    }
  }

  Stream<Uint8List> getImageStream() async* {
    while (true) {
      try {
        final imageData = await getStreamFrame();
        if (imageData != null) {
          yield imageData;
        }
        await Future.delayed(const Duration(milliseconds: 100)); // 10 FPS
      } catch (e) {
        debugPrint('Error en stream ESP32: $e');
        await Future.delayed(const Duration(seconds: 1));
      }
    }
  }
}