import 'package:flutter/material.dart';
import 'package:camera/camera.dart';
import 'dart:io';
import '../widgets/animated_widgets.dart';
import '../services/camera_service.dart';
import '../services/api_service.dart';

class CameraScreen extends StatefulWidget {
  const CameraScreen({super.key});

  @override
  State<CameraScreen> createState() => _CameraScreenState();
}

class _CameraScreenState extends State<CameraScreen> {
  final CameraService _cameraService = CameraService();
  final ApiService _apiService = ApiService();
  
  bool _isLoading = false;
  String _detectedObjects = '0';
  String _confidence = '0.0%';
  String _lastDetectedMaterial = 'Ninguno';
  PredictionResult? _lastPrediction;

  @override
  void initState() {
    super.initState();
    _initializeCamera();
  }

  @override
  void dispose() {
    _cameraService.dispose();
    super.dispose();
  }

  Future<void> _initializeCamera() async {
    try {
      await _cameraService.initialize();
      if (mounted) setState(() {});
    } catch (e) {
      _showError('Error inicializando cámara: $e');
    }
  }

  Future<void> _takePictureAndClassify() async {
    if (_isLoading) return;
    
    setState(() => _isLoading = true);
    
    try {
      final image = await _cameraService.takePicture();
      if (image != null) {
        final prediction = await _cameraService.classifyImage(image);
        if (prediction != null) {
          setState(() {
            _lastPrediction = prediction;
            _detectedObjects = '1';
            _confidence = prediction.confidencePercentage;
            _lastDetectedMaterial = prediction.materialType;
          });
          _showPredictionResult(prediction);
        } else {
          _showError('Error al clasificar la imagen');
        }
      }
    } catch (e) {
      _showError('Error: $e');
    } finally {
      setState(() => _isLoading = false);
    }
  }

  void _showPredictionResult(PredictionResult prediction) {
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        backgroundColor: const Color(0xFF1a1a2e),
        title: const Text(
          'Resultado de Clasificación',
          style: TextStyle(color: Color(0xFF00aaff)),
        ),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              'Material: ${prediction.materialType}',
              style: const TextStyle(
                color: Colors.white,
                fontSize: 18,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 8),
            Text(
              'Confianza: ${prediction.confidencePercentage}',
              style: const TextStyle(color: Colors.white70),
            ),
            const SizedBox(height: 16),
            const Text(
              'Probabilidades:',
              style: TextStyle(
                color: Color(0xFF00aaff),
                fontWeight: FontWeight.bold,
              ),
            ),
            ...prediction.allProbabilities.entries.map((entry) {
              final materialName = _getMaterialName(entry.key);
              final percentage = (entry.value * 100).toStringAsFixed(1);
              return Padding(
                padding: const EdgeInsets.symmetric(vertical: 2),
                child: Text(
                  '$materialName: $percentage%',
                  style: const TextStyle(color: Colors.white70),
                ),
              );
            }),
          ],
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text(
              'OK',
              style: TextStyle(color: Color(0xFF00aaff)),
            ),
          ),
        ],
      ),
    );
  }

  String _getMaterialName(String key) {
    switch (key.toLowerCase()) {
      case 'glass':
        return 'Vidrio';
      case 'plastic':
        return 'Plástico';
      case 'metal':
        return 'Metal';
      default:
        return key;
    }
  }

  void _showError(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(message),
        backgroundColor: Colors.red,
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
      padding: const EdgeInsets.all(16),
      child: Column(
        children: [
          SlideInAnimation(
            begin: const Offset(0, -0.3),
            duration: const Duration(milliseconds: 800),
            child: _buildCameraView(),
          ),
          const SizedBox(height: 16),
          SlideInAnimation(
            begin: const Offset(-0.3, 0),
            duration: const Duration(milliseconds: 900),
            delay: const Duration(milliseconds: 200),
            child: _buildCameraConfig(),
          ),
          const SizedBox(height: 16),
          SlideInAnimation(
            begin: const Offset(0.3, 0),
            duration: const Duration(milliseconds: 1000),
            delay: const Duration(milliseconds: 400),
            child: _buildDetectionInfo(),
          ),
        ],
      ),
    );
  }

  Widget _buildCameraView() {
    return AnimatedCard(
      child: Column(
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              const PulsingDot(
                color: Colors.green,
                size: 12,
                duration: Duration(milliseconds: 800),
              ),
              const SizedBox(width: 16),
              const SlideInAnimation(
                begin: Offset(0.3, 0),
                duration: Duration(milliseconds: 1000),
                delay: Duration(milliseconds: 300),
                child: Text(
                  'EN VIVO - 1080p',
                  style: TextStyle(
                    color: Colors.green,
                    fontSize: 16,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ),
            ],
          ),
          const SizedBox(height: 16),
          SlideInAnimation(
            begin: const Offset(0, 0.2),
            duration: const Duration(milliseconds: 1200),
            delay: const Duration(milliseconds: 500),
            child: Container(
              width: double.infinity,
              height: 300,
              decoration: BoxDecoration(
                gradient: const LinearGradient(
                  colors: [Color(0xFF1e3c72), Color(0xFF2a5298)],
                  begin: Alignment.topLeft,
                  end: Alignment.bottomRight,
                ),
                borderRadius: BorderRadius.circular(8),
                border: Border.all(color: Colors.white.withOpacity(0.2), width: 2),
              ),
              child: ClipRRect(
                borderRadius: BorderRadius.circular(8),
                child: _cameraService.isInitialized && _cameraService.controller != null
                    ? CameraPreview(_cameraService.controller!)
                    : const Center(
                        child: Column(
                          mainAxisAlignment: MainAxisAlignment.center,
                          children: [
                            CircularProgressIndicator(
                              color: Colors.white70,
                            ),
                            SizedBox(height: 16),
                            Text(
                              'Inicializando Cámara...',
                              style: TextStyle(
                                color: Colors.white70,
                                fontSize: 18,
                              ),
                            ),
                          ],
                        ),
                      ),
              ),
            ),
          ),
          const SizedBox(height: 16),
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 800),
            delay: const Duration(milliseconds: 800),
            child: Row(
              children: [
                Expanded(
                  child: FloatingButton(
                    duration: const Duration(seconds: 4),
                    child: _buildControlButton('[+] ZOOM'),
                  ),
                ),
                const SizedBox(width: 8),
                Expanded(
                  child: FloatingButton(
                    duration: const Duration(seconds: 5),
                    child: _buildControlButton('[-] ZOOM'),
                  ),
                ),
                const SizedBox(width: 8),
                Expanded(
                  child: FloatingButton(
                    duration: const Duration(seconds: 3),
                    child: GestureDetector(
                      onTap: _takePictureAndClassify,
                      child: _buildControlButton(
                        _isLoading ? 'ANALIZANDO...' : '[*] CLASIFICAR'
                      ),
                    ),
                  ),
                ),
                const SizedBox(width: 8),
                Expanded(
                  child: FloatingButton(
                    duration: const Duration(seconds: 6),
                    child: _buildControlButton('[F] FOCO'),
                  ),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildControlButton(String text) {
    return Container(
      height: 50,
      padding: const EdgeInsets.all(12),
      decoration: BoxDecoration(
        color: Colors.black.withOpacity(0.5),
        border: Border.all(color: Colors.white.withOpacity(0.3), width: 2),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Center(
        child: Text(
          text,
          style: const TextStyle(
            color: Colors.white,
            fontSize: 14,
            fontWeight: FontWeight.bold,
          ),
        ),
      ),
    );
  }

  Widget _buildCameraConfig() {
    return AnimatedCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            '[SET] Config Cámara',
            style: TextStyle(
              color: Color(0xFF00aaff),
              fontSize: 18,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 16),
          Row(
            children: [
              Expanded(
                child: SlideInAnimation(
                  begin: const Offset(-0.3, 0),
                  duration: const Duration(milliseconds: 800),
                  delay: const Duration(milliseconds: 400),
                  child: Column(
                    children: [
                      AnimatedProgressBar(
                        label: '[F] Enfoque',
                        value: '87%',
                        progress: 0.87,
                        color: const Color(0xFF4ecdc4),
                        duration: const Duration(milliseconds: 2000),
                      ),
                      const SizedBox(height: 12),
                      Row(
                        children: [
                          Expanded(
                            child: FloatingButton(
                              duration: const Duration(seconds: 3),
                              child: _buildActionButton('MENOS'),
                            ),
                          ),
                          const SizedBox(width: 8),
                          Expanded(
                            child: FloatingButton(
                              duration: const Duration(seconds: 4),
                              child: _buildActionButton('MÁS'),
                            ),
                          ),
                        ],
                      ),
                    ],
                  ),
                ),
              ),
              const SizedBox(width: 16),
              Expanded(
                child: SlideInAnimation(
                  begin: const Offset(0.3, 0),
                  duration: const Duration(milliseconds: 800),
                  delay: const Duration(milliseconds: 600),
                  child: Column(
                    children: [
                      AnimatedProgressBar(
                        label: '[L] Exposición',
                        value: '50%',
                        progress: 0.50,
                        color: const Color(0xFFfeca57),
                        duration: const Duration(milliseconds: 2500),
                      ),
                      const SizedBox(height: 12),
                      Row(
                        children: [
                          Expanded(
                            child: FloatingButton(
                              duration: const Duration(seconds: 5),
                              child: _buildControlButton('[D] OSCURO'),
                            ),
                          ),
                          const SizedBox(width: 8),
                          Expanded(
                            child: FloatingButton(
                              duration: const Duration(seconds: 3),
                              child: _buildControlButton('[B] CLARO'),
                            ),
                          ),
                        ],
                      ),
                    ],
                  ),
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildActionButton(String text) {
    return Container(
      height: 40,
      decoration: BoxDecoration(
        gradient: const LinearGradient(
          colors: [Color(0xFF667eea), Color(0xFF764ba2)],
        ),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Center(
        child: Text(
          text,
          style: const TextStyle(
            color: Colors.white,
            fontSize: 14,
            fontWeight: FontWeight.bold,
          ),
        ),
      ),
    );
  }

  Widget _buildDetectionInfo() {
    return AnimatedCard(
      child: Column(
        children: [
          const Text(
            '[T] Detección Activa',
            style: TextStyle(
              color: Color(0xFF00aaff),
              fontSize: 18,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 20),
          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              const PulsingDot(
                color: Color(0xFF00aaff),
                size: 12,
                duration: Duration(seconds: 1),
              ),
              const SizedBox(width: 12),
              SlideInAnimation(
                begin: const Offset(0.3, 0),
                duration: const Duration(milliseconds: 1000),
                delay: const Duration(milliseconds: 600),
                child: Text(
                  'Material: $_lastDetectedMaterial',
                  style: const TextStyle(
                    color: Color(0xFF00aaff),
                    fontSize: 20,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ),
            ],
          ),
          const SizedBox(height: 8),
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 800),
            delay: const Duration(milliseconds: 800),
            child: Text(
              'Confianza: $_confidence',
              style: const TextStyle(
                color: Colors.grey,
                fontSize: 18,
              ),
            ),
          ),
        ],
      ),
    );
  }

}