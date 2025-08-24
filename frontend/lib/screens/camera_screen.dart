import 'package:flutter/material.dart';
import 'dart:typed_data';
import 'dart:async';
import '../widgets/animated_widgets.dart';
import '../widgets/loading_widgets.dart';
import '../services/system_service.dart';
import '../services/api_service.dart';

class CameraScreen extends StatefulWidget {
  const CameraScreen({super.key});

  @override
  State<CameraScreen> createState() => _CameraScreenState();
}

class _CameraScreenState extends State<CameraScreen> {
  final SystemService _systemService = SystemService();
  
  bool _isLoading = false;
  String _detectedObjects = '0';
  String _confidence = '0.0%';
  String _lastDetectedMaterial = 'Ninguno';
  PredictionResult? _lastPrediction;
  Uint8List? _currentFrame;
  StreamSubscription<Uint8List>? _streamSubscription;
  bool _isStreamActive = false;
  bool _flashEnabled = false;
  int _imageQuality = 12;

  @override
  void initState() {
    super.initState();
    _startImageStream();
  }

  @override
  void dispose() {
    _streamSubscription?.cancel();
    super.dispose();
  }

  void _startImageStream() {
    if (!_isStreamActive) {
      _isStreamActive = true;
      
      setState(() {
        _isLoading = true;
      });
      
      // Optimizado para Raspberry Pi 5 - menor frecuencia de captura
      Timer.periodic(const Duration(milliseconds: 500), (timer) async {
        if (!_isStreamActive || !mounted) {
          timer.cancel();
          return;
        }
        
        try {
          final imageBytes = await _systemService.captureImageFromEsp32();
          if (imageBytes != null && mounted) {
            WidgetsBinding.instance.addPostFrameCallback((_) {
              if (mounted) {
                setState(() {
                  _currentFrame = imageBytes;
                  _isLoading = false; // Ocultar loading cuando se carga la primera imagen
                });
              }
            });
          }
        } catch (e) {
          // Error silencioso para no saturar UI
          if (mounted) {
            WidgetsBinding.instance.addPostFrameCallback((_) {
              if (mounted) {
                setState(() {
                  _isLoading = false;
                });
              }
            });
          }
        }
      });
    }
  }

  void _stopImageStream() {
    _streamSubscription?.cancel();
    _isStreamActive = false;
  }

  Future<void> _takePictureAndClassify() async {
    if (_isLoading) return;
    
    if (mounted) {
      setState(() => _isLoading = true);
    }
    
    try {
      final imageBytes = await _systemService.captureImageFromEsp32();
      if (imageBytes != null) {
        final prediction = await _systemService.classifyImage(imageBytes);
        if (prediction != null && mounted) {
          WidgetsBinding.instance.addPostFrameCallback((_) {
            if (mounted) {
              setState(() {
                _lastPrediction = prediction;
                _detectedObjects = '1';
                _confidence = prediction.confidencePercentage;
                _lastDetectedMaterial = prediction.materialType;
              });
            }
          });
          _showPredictionResult(prediction);
        } else {
          _showError('Error al clasificar la imagen');
        }
      } else {
        _showError('Error capturando imagen desde ESP32');
      }
    } catch (e) {
      _showError('Error: $e');
    } finally {
      if (mounted) {
        WidgetsBinding.instance.addPostFrameCallback((_) {
          if (mounted) {
            setState(() => _isLoading = false);
          }
        });
      }
    }
  }

  Future<void> _toggleFlash() async {
    _flashEnabled = !_flashEnabled;
    final success = await _systemService.controlEsp32Cam(flashEnabled: _flashEnabled);
    if (!success) {
      _flashEnabled = !_flashEnabled;
      _showError('Error controlando flash ESP32-CAM');
    }
    if (mounted) {
      setState(() {});
    }
  }

  Future<void> _adjustQuality(bool increase) async {
    int newQuality = _imageQuality + (increase ? 5 : -5);
    newQuality = newQuality.clamp(0, 63);
    
    final success = await _systemService.controlEsp32Cam(quality: newQuality);
    if (success && mounted) {
      setState(() {
        _imageQuality = newQuality;
      });
    } else {
      _showError('Error ajustando calidad ESP32-CAM');
    }
  }

  Future<void> _restartCamera() async {
    _stopImageStream();
    final success = await _systemService.controlEsp32Cam(action: 'restart');
    if (success) {
      await Future.delayed(const Duration(seconds: 2));
      _startImageStream();
    } else {
      _showError('Error reiniciando ESP32-CAM');
      _startImageStream();
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
                child: _currentFrame != null
                    ? Image.memory(
                        _currentFrame!,
                        fit: BoxFit.cover,
                        width: double.infinity,
                        height: double.infinity,
                        // Optimización para Raspberry Pi 5
                        filterQuality: FilterQuality.medium,
                      )
                    : const RaspberryPiLoader(
                        message: 'Conectando con ESP32-CAM...',
                        size: 48,
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
                    child: GestureDetector(
                      onTap: () => _adjustQuality(true),
                      child: _buildControlButton('[+] CALIDAD'),
                    ),
                  ),
                ),
                const SizedBox(width: 8),
                Expanded(
                  child: FloatingButton(
                    duration: const Duration(seconds: 5),
                    child: GestureDetector(
                      onTap: () => _adjustQuality(false),
                      child: _buildControlButton('[-] CALIDAD'),
                    ),
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
                    child: GestureDetector(
                      onTap: _toggleFlash,
                      child: _buildControlButton(
                        _flashEnabled ? '[F] FLASH ON' : '[F] FLASH OFF'
                      ),
                    ),
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
      decoration: BoxDecoration(
        gradient: const LinearGradient(
          colors: [Color(0xFF1a1a2e), Color(0xFF16213e)],
          begin: Alignment.topLeft,
          end: Alignment.bottomRight,
        ),
        borderRadius: BorderRadius.circular(12),
        border: Border.all(
          color: const Color(0xFF00aaff).withOpacity(0.3),
          width: 1,
        ),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withOpacity(0.3),
            blurRadius: 8,
            offset: const Offset(0, 4),
          ),
        ],
      ),
      child: Center(
        child: Text(
          text,
          style: const TextStyle(
            color: Colors.white,
            fontSize: 12,
            fontWeight: FontWeight.bold,
            letterSpacing: 0.5,
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
                        label: '[Q] Calidad',
                        value: '${((_imageQuality / 63) * 100).toInt()}%',
                        progress: _imageQuality / 63,
                        color: const Color(0xFF4ecdc4),
                        duration: const Duration(milliseconds: 2000),
                      ),
                      const SizedBox(height: 12),
                      Row(
                        children: [
                          Expanded(
                            child: FloatingButton(
                              duration: const Duration(seconds: 3),
                              child: GestureDetector(
                                onTap: () => _adjustQuality(false),
                                child: _buildActionButton('MENOR'),
                              ),
                            ),
                          ),
                          const SizedBox(width: 8),
                          Expanded(
                            child: FloatingButton(
                              duration: const Duration(seconds: 4),
                              child: GestureDetector(
                                onTap: () => _adjustQuality(true),
                                child: _buildActionButton('MAYOR'),
                              ),
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
                        label: '[S] Estado ESP32-CAM',
                        value: _isStreamActive ? 'CONECTADO' : 'DESCONECTADO',
                        progress: _isStreamActive ? 1.0 : 0.0,
                        color: const Color(0xFFfeca57),
                        duration: const Duration(milliseconds: 2500),
                      ),
                      const SizedBox(height: 12),
                      Row(
                        children: [
                          Expanded(
                            child: FloatingButton(
                              duration: const Duration(seconds: 5),
                              child: GestureDetector(
                                onTap: _restartCamera,
                                child: _buildControlButton('[R] REINICIAR'),
                              ),
                            ),
                          ),
                          const SizedBox(width: 8),
                          Expanded(
                            child: FloatingButton(
                              duration: const Duration(seconds: 3),
                              child: GestureDetector(
                                onTap: _isStreamActive ? _stopImageStream : _startImageStream,
                                child: _buildControlButton(
                                  _isStreamActive ? '[P] PAUSAR' : '[I] INICIAR'
                                ),
                              ),
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
          colors: [Color(0xFF00aaff), Color(0xFF0099cc)],
          begin: Alignment.topLeft,
          end: Alignment.bottomRight,
        ),
        borderRadius: BorderRadius.circular(12),
        boxShadow: [
          BoxShadow(
            color: const Color(0xFF00aaff).withOpacity(0.3),
            blurRadius: 8,
            offset: const Offset(0, 4),
          ),
        ],
      ),
      child: Center(
        child: Text(
          text,
          style: const TextStyle(
            color: Colors.white,
            fontSize: 12,
            fontWeight: FontWeight.bold,
            letterSpacing: 0.5,
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