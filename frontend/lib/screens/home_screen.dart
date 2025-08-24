import 'package:flutter/material.dart';
import 'dart:async';
import '../widgets/animated_widgets.dart';
import '../widgets/loading_widgets.dart';
import '../services/system_service.dart';
import '../services/app_state_service.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> with AppStateMixin {
  final SystemService _systemService = SystemService();
  SystemStatus? _systemStatus;
  SystemInfo? _systemInfo;
  SystemMetrics? _systemMetrics;
  SensorData? _sensorData;
  List<PirSensorData>? _pirSensors;
  List<WeightSensorData>? _weightSensors;
  Esp32CamStatus? _esp32CamStatus;
  Esp32ControlStatus? _esp32ControlStatus;
  bool _isLoading = true;
  Timer? _updateTimer;

  @override
  void initState() {
    super.initState();
    _loadSystemData();
    _startPeriodicUpdate();
  }

  @override
  void dispose() {
    _updateTimer?.cancel();
    super.dispose();
  }

  void _startPeriodicUpdate() {
    // Optimizado para Raspberry Pi 5 - menos frecuente para ahorrar CPU
    _updateTimer = Timer.periodic(const Duration(seconds: 8), (timer) {
      if (mounted) {
        _loadSystemData();
      }
    });
  }

  Future<void> _loadSystemData() async {
    try {
      final futures = await Future.wait([
        _systemService.getSystemStatus().catchError((e) => null),
        _systemService.getSystemInfo().catchError((e) => null),
        _systemService.getSystemMetrics().catchError((e) => null),
        _systemService.getSensorData().catchError((e) => null),
        _systemService.getAllPirSensors().catchError((e) => null),
        _systemService.getAllWeightSensors().catchError((e) => null),
        _systemService.getEsp32CamStatus().catchError((e) => null),
        _systemService.getEsp32ControlStatus().catchError((e) => null),
      ]);

      // Usar WidgetsBinding para evitar setState durante layout
      if (mounted) {
        WidgetsBinding.instance.addPostFrameCallback((_) {
          if (mounted) {
            setState(() {
              _systemStatus = futures[0] as SystemStatus?;
              _systemInfo = futures[1] as SystemInfo?;
              _systemMetrics = futures[2] as SystemMetrics?;
              _sensorData = futures[3] as SensorData?;
              _pirSensors = futures[4] as List<PirSensorData>?;
              _weightSensors = futures[5] as List<WeightSensorData>?;
              _esp32CamStatus = futures[6] as Esp32CamStatus?;
              _esp32ControlStatus = futures[7] as Esp32ControlStatus?;
              _isLoading = false;
            });
          }
        });
      }
    } catch (e) {
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
  }

  @override
  Widget build(BuildContext context) {
    if (_isLoading) {
      return const RaspberryPiLoader(
        message: 'Cargando datos del sistema...',
        size: 64,
      );
    }

    return SingleChildScrollView(
      padding: const EdgeInsets.all(16),
      child: Column(
        children: [
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 800),
            child: Row(
              children: [
                Expanded(
                  child: _buildSystemCard(),
                ),
                const SizedBox(width: 16),
                Expanded(
                  child: _buildDetectionCard(),
                ),
              ],
            ),
          ),
          const SizedBox(height: 16),
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 900),
            delay: const Duration(milliseconds: 200),
            child: _buildMaterialCard(),
          ),
          const SizedBox(height: 16),
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 1000),
            delay: const Duration(milliseconds: 400),
            child: _buildAICard(),
          ),
          const SizedBox(height: 16),
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 1100),
            delay: const Duration(milliseconds: 600),
            child: _buildSensorsCard(),
          ),
        ],
      ),
    );
  }

  Widget _buildSystemCard() {
    return AnimatedCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            '[PC] Sistema',
            style: TextStyle(
              color: Color(0xFF00aaff),
              fontSize: 18,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 16),
          if (_systemMetrics != null) ...[
            OptimizedProgressBar(
              label: 'CPU: ${_systemMetrics!.cpuUsage.toInt()}%',
              progress: _systemMetrics!.cpuUsage / 100,
              color: const Color(0xFFFF6B6B),
            ),
            const SizedBox(height: 12),
            OptimizedProgressBar(
              label: 'RAM: ${_systemMetrics!.memoryUsage.toInt()}%',
              progress: _systemMetrics!.memoryUsage / 100,
              color: const Color(0xFF4ECDC4),
            ),
            const SizedBox(height: 12),
            OptimizedProgressBar(
              label: 'Disco: ${_systemMetrics!.diskUsage.toInt()}%',
              progress: _systemMetrics!.diskUsage / 100,
              color: const Color(0xFFFECA57),
            ),
          ] else ...[
            const SkeletonLoader(width: double.infinity, height: 24),
            const SizedBox(height: 12),
            const SkeletonLoader(width: double.infinity, height: 24),
            const SizedBox(height: 12),
            const SkeletonLoader(width: double.infinity, height: 24),
          ],
        ],
      ),
    );
  }

  Widget _buildDetectionCard() {
    final detectedMaterial = appState.lastDetectedMaterial ?? 'Sin detección';
    final confidence = appState.lastConfidence;
    final targetMaterial = appState.targetMaterial;
    final materialColor = appState.getMaterialColor(detectedMaterial);
    final isMatch = appState.doesMaterialMatch(detectedMaterial);
    
    return AnimatedCard(
      child: Column(
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              const Text(
                '[%] Detección',
                style: TextStyle(
                  color: Color(0xFF00aaff),
                  fontSize: 18,
                  fontWeight: FontWeight.bold,
                ),
              ),
              Container(
                padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 4),
                decoration: BoxDecoration(
                  color: appState.getMaterialColor(targetMaterial).withOpacity(0.2),
                  borderRadius: BorderRadius.circular(12),
                  border: Border.all(
                    color: appState.getMaterialColor(targetMaterial),
                    width: 1,
                  ),
                ),
                child: Text(
                  'Objetivo: $targetMaterial',
                  style: TextStyle(
                    color: appState.getMaterialColor(targetMaterial),
                    fontSize: 10,
                    fontWeight: FontWeight.w600,
                  ),
                ),
              ),
            ],
          ),
          const SizedBox(height: 20),
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 1500),
            delay: const Duration(milliseconds: 800),
            child: Column(
              children: [
                Text(
                  detectedMaterial,
                  style: TextStyle(
                    color: materialColor,
                    fontSize: 32,
                    fontWeight: FontWeight.bold,
                  ),
                ),
                if (isMatch && detectedMaterial != 'Sin detección')
                  Container(
                    margin: const EdgeInsets.only(top: 4),
                    padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 2),
                    decoration: BoxDecoration(
                      color: const Color(0xFF00C851).withOpacity(0.2),
                      borderRadius: BorderRadius.circular(8),
                    ),
                    child: const Text(
                      '✓ MATCH',
                      style: TextStyle(
                        color: Color(0xFF00C851),
                        fontSize: 12,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                  ),
              ],
            ),
          ),
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 1200),
            delay: const Duration(milliseconds: 1200),
            child: Text(
              confidence,
              style: const TextStyle(
                color: Colors.grey,
                fontSize: 20,
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildMaterialCard() {
    return AnimatedCard(
      width: double.infinity,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            '[#] Materiales',
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
                  child: _buildMaterialItem('45%', 'Plástico', const Color(0xFF4ecdc4)),
                ),
              ),
              Expanded(
                child: SlideInAnimation(
                  begin: const Offset(0, -0.3),
                  duration: const Duration(milliseconds: 800),
                  delay: const Duration(milliseconds: 600),
                  child: _buildMaterialItem('30%', 'Vidrio', Colors.red),
                ),
              ),
              Expanded(
                child: SlideInAnimation(
                  begin: const Offset(0.3, 0),
                  duration: const Duration(milliseconds: 800),
                  delay: const Duration(milliseconds: 800),
                  child: _buildMaterialItem('25%', 'Cartón', const Color(0xFFfeca57)),
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildMaterialItem(String percentage, String type, Color color) {
    return Container(
      padding: const EdgeInsets.all(12),
      margin: const EdgeInsets.symmetric(horizontal: 4),
      decoration: BoxDecoration(
        color: Colors.black.withOpacity(0.4),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Column(
        children: [
          Text(
            percentage,
            style: TextStyle(
              color: color,
              fontSize: 24,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 6),
          Text(
            type.toUpperCase(),
            style: const TextStyle(
              color: Colors.white,
              fontSize: 14,
              fontWeight: FontWeight.bold,
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildAICard() {
    return AnimatedCard(
      width: double.infinity,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            '[AI] Inteligencia Artificial',
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
                  delay: const Duration(milliseconds: 600),
                  child: Container(
                    padding: const EdgeInsets.all(16),
                    decoration: BoxDecoration(
                      color: Colors.black.withOpacity(0.4),
                      borderRadius: BorderRadius.circular(8),
                      border: Border.all(
                        color: _systemStatus != null && _systemStatus!.isActive 
                            ? const Color(0xFF00C851) 
                            : const Color(0xFFFF3547), 
                        width: 2
                      ),
                      boxShadow: [
                        BoxShadow(
                          color: (_systemStatus != null && _systemStatus!.isActive 
                              ? const Color(0xFF00C851) 
                              : const Color(0xFFFF3547)).withOpacity(0.3),
                          blurRadius: 8,
                        ),
                      ],
                    ),
                    child: Column(
                      children: [
                        const Text(
                          'CNN',
                          style: TextStyle(
                            color: Colors.white,
                            fontSize: 18,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                        const SizedBox(height: 8),
                        Text(
                          _systemStatus != null && _systemStatus!.isActive ? 'Activo' : 'Inactivo',
                          style: TextStyle(
                            color: _systemStatus != null && _systemStatus!.isActive ? const Color(0xFF00aaff) : const Color(0xFFFF3547),
                            fontSize: 16,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                      ],
                    ),
                  ),
                ),
              ),
              const SizedBox(width: 12),
              Expanded(
                child: SlideInAnimation(
                  begin: const Offset(0.3, 0),
                  duration: const Duration(milliseconds: 800),
                  delay: const Duration(milliseconds: 800),
                  child: Container(
                    padding: const EdgeInsets.all(16),
                    decoration: BoxDecoration(
                      color: Colors.black.withOpacity(0.4),
                      borderRadius: BorderRadius.circular(8),
                      border: Border.all(
                        color: _systemStatus?.components['rnn']?['connected'] == true 
                            ? const Color(0xFF00C851) 
                            : const Color(0xFFFF3547), 
                        width: 2
                      ),
                      boxShadow: [
                        BoxShadow(
                          color: (_systemStatus?.components['rnn']?['connected'] == true 
                              ? const Color(0xFF00C851) 
                              : const Color(0xFFFF3547)).withOpacity(0.3),
                          blurRadius: 8,
                        ),
                      ],
                    ),
                    child: Column(
                      children: [
                        const Text(
                          'RNN',
                          style: TextStyle(
                            color: Colors.white,
                            fontSize: 18,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                        const SizedBox(height: 8),
                        Text(
                          _systemStatus?.components['rnn']?['accuracy']?.toString() ?? 'N/A',
                          style: TextStyle(
                            color: _systemStatus?.components['rnn']?['connected'] == true 
                                ? const Color(0xFF00aaff) 
                                : const Color(0xFFFF3547),
                            fontSize: 16,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                      ],
                    ),
                  ),
                ),
              ),
            ],
          ),
          const SizedBox(height: 16),
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 800),
            delay: const Duration(milliseconds: 1000),
            child: Container(
              padding: const EdgeInsets.all(12),
              decoration: BoxDecoration(
                color: Colors.black.withOpacity(0.4),
                borderRadius: BorderRadius.circular(8),
              ),
              child: Column(
                children: [
                  Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      Text(
                        _systemStatus != null ? 'FastAPI Conectado' : 'FastAPI Desconectado',
                        style: const TextStyle(color: Colors.white, fontSize: 14),
                      ),
                      PulsingDot(
                        color: _systemStatus != null ? Colors.green : Colors.red,
                        size: 12,
                        duration: const Duration(seconds: 1),
                      ),
                    ],
                  ),
                  const SizedBox(height: 6),
                  Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      Text(
                        _systemStatus != null ? 'Estado: ${_systemStatus!.status}' : 'Estado: Offline',
                        style: const TextStyle(color: Colors.white, fontSize: 14),
                      ),
                      Text(
                        _systemInfo != null ? 'v${_systemInfo!.version}' : 'N/A',
                        style: const TextStyle(color: Color(0xFF00aaff), fontSize: 14),
                      ),
                    ],
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildSensorsCard() {
    return AnimatedCard(
      width: double.infinity,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            '[S] Sensores PIR',
            style: TextStyle(
              color: Color(0xFF00aaff),
              fontSize: 18,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 16),
          Row(
            children: [
              if (_pirSensors != null && _pirSensors!.isNotEmpty) ...[
                for (int i = 0; i < 3 && i < _pirSensors!.length; i++)
                  Expanded(
                    child: SlideInAnimation(
                      begin: Offset(-0.3 + (i * 0.3), 0),
                      duration: const Duration(milliseconds: 600),
                      delay: Duration(milliseconds: 200 + (i * 200)),
                      child: _buildSensor(
                        _pirSensors![i].id, 
                        _pirSensors![i].isActive ? 'OK' : 'ERR', 
                        _pirSensors![i].isActive
                      ),
                    ),
                  ),
              ] else ...[
                // Fallback si no hay datos del backend
                Expanded(
                  child: SlideInAnimation(
                    begin: const Offset(-0.3, 0),
                    duration: const Duration(milliseconds: 600),
                    delay: const Duration(milliseconds: 200),
                    child: _buildSensor('PIR-1', 'ERR', false),
                  ),
                ),
                Expanded(
                  child: SlideInAnimation(
                    begin: const Offset(0, -0.3),
                    duration: const Duration(milliseconds: 600),
                    delay: const Duration(milliseconds: 400),
                    child: _buildSensor('PIR-2', 'ERR', false),
                  ),
                ),
                Expanded(
                  child: SlideInAnimation(
                    begin: const Offset(0.3, 0),
                    duration: const Duration(milliseconds: 600),
                    delay: const Duration(milliseconds: 600),
                    child: _buildSensor('PIR-3', 'ERR', false),
                  ),
                ),
              ],
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildSensor(String label, String value, bool isOk) {
    return AnimatedCard(
      padding: const EdgeInsets.all(8),
      child: SizedBox(
        height: 64,
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Text(
              label,
              style: const TextStyle(
                color: Colors.white,
                fontSize: 14,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 6),
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                PulsingDot(
                  color: isOk ? const Color(0xFF00C851) : const Color(0xFFFF3547),
                  size: 8,
                  duration: isOk ? const Duration(seconds: 2) : const Duration(milliseconds: 500),
                ),
                const SizedBox(width: 8),
                Text(
                  value,
                  style: TextStyle(
                    color: isOk ? const Color(0xFF00C851) : const Color(0xFFFF3547),
                    fontSize: 16,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}