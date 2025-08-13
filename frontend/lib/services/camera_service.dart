import 'dart:io';
import 'package:camera/camera.dart';
import 'package:flutter/foundation.dart';
import 'package:image_picker/image_picker.dart';
import 'package:path_provider/path_provider.dart';
import 'package:permission_handler/permission_handler.dart';
import 'api_service.dart';

class CameraService {
  static final CameraService _instance = CameraService._internal();
  factory CameraService() => _instance;
  CameraService._internal();

  CameraController? _controller;
  List<CameraDescription>? _cameras;
  bool _isInitialized = false;
  final ImagePicker _picker = ImagePicker();

  CameraController? get controller => _controller;
  bool get isInitialized => _isInitialized;

  Future<void> initialize() async {
    try {
      // Solicitar permisos
      final cameraStatus = await Permission.camera.request();
      if (!cameraStatus.isGranted) {
        throw Exception('Permisos de cámara no otorgados');
      }

      // Obtener cámaras disponibles
      _cameras = await availableCameras();
      if (_cameras!.isEmpty) {
        throw Exception('No hay cámaras disponibles');
      }

      // Inicializar controlador con la cámara trasera
      final camera = _cameras!.firstWhere(
        (camera) => camera.lensDirection == CameraLensDirection.back,
        orElse: () => _cameras!.first,
      );

      _controller = CameraController(
        camera,
        ResolutionPreset.high,
        enableAudio: false,
      );

      await _controller!.initialize();
      _isInitialized = true;
    } catch (e) {
      debugPrint('Error inicializando cámara: $e');
      _isInitialized = false;
      rethrow;
    }
  }

  Future<File?> takePicture() async {
    if (!_isInitialized || _controller == null) {
      throw Exception('Cámara no inicializada');
    }

    try {
      final XFile image = await _controller!.takePicture();
      return File(image.path);
    } catch (e) {
      debugPrint('Error tomando foto: $e');
      return null;
    }
  }

  Future<File?> pickImageFromGallery() async {
    try {
      final XFile? image = await _picker.pickImage(
        source: ImageSource.gallery,
        imageQuality: 80,
      );
      return image != null ? File(image.path) : null;
    } catch (e) {
      debugPrint('Error seleccionando imagen: $e');
      return null;
    }
  }

  Future<List<File>?> pickMultipleImages() async {
    try {
      final List<XFile> images = await _picker.pickMultiImage(
        imageQuality: 80,
      );
      return images.map((image) => File(image.path)).toList();
    } catch (e) {
      debugPrint('Error seleccionando imágenes múltiples: $e');
      return null;
    }
  }

  Future<PredictionResult?> classifyImage(File imageFile) async {
    try {
      final apiService = ApiService();
      final result = await apiService.uploadImage(imageFile.path);
      return PredictionResult.fromJson(result);
    } catch (e) {
      debugPrint('Error clasificando imagen: $e');
      return null;
    }
  }

  Future<List<PredictionResult>?> classifyMultipleImages(
      List<File> imageFiles) async {
    try {
      final apiService = ApiService();
      final imagePaths = imageFiles.map((file) => file.path).toList();
      final results = await apiService.uploadMultipleImages(imagePaths);
      
      return results.map((result) {
        // Para batch predictions, la estructura puede ser ligeramente diferente
        return PredictionResult(
          predictedClass: result['predicted_class'],
          confidence: result['confidence'].toDouble(),
          allProbabilities: Map<String, double>.from(
            result['all_probabilities'].map((k, v) => MapEntry(k, v.toDouble())),
          ),
          timestamp: DateTime.now(),
          filename: result['filename'],
        );
      }).toList();
    } catch (e) {
      debugPrint('Error clasificando múltiples imágenes: $e');
      return null;
    }
  }

  Future<File> saveImageToAppDirectory(File image) async {
    final appDir = await getApplicationDocumentsDirectory();
    final fileName = 'captured_${DateTime.now().millisecondsSinceEpoch}.jpg';
    final savedImage = await image.copy('${appDir.path}/$fileName');
    return savedImage;
  }

  void switchCamera() {
    if (_cameras != null && _cameras!.length > 1 && _controller != null) {
      final currentCamera = _controller!.description;
      final newCamera = _cameras!.firstWhere(
        (camera) => camera != currentCamera,
        orElse: () => _cameras!.first,
      );
      
      _initializeCamera(newCamera);
    }
  }

  Future<void> _initializeCamera(CameraDescription camera) async {
    await _controller?.dispose();
    
    _controller = CameraController(
      camera,
      ResolutionPreset.high,
      enableAudio: false,
    );

    await _controller!.initialize();
  }

  void dispose() {
    _controller?.dispose();
    _isInitialized = false;
  }
}