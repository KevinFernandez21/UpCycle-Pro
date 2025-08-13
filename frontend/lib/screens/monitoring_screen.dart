import 'package:flutter/material.dart';
import 'dart:async';
import '../widgets/animated_widgets.dart';
import '../services/microcontroller_service.dart';
import '../services/config_service.dart';

class MonitoringScreen extends StatefulWidget {
  const MonitoringScreen({super.key});

  @override
  State<MonitoringScreen> createState() => _MonitoringScreenState();
}

class _MonitoringScreenState extends State<MonitoringScreen> {
  final MicrocontrollerService _microcontrollerService = MicrocontrollerService();
  
  Timer? _updateTimer;
  SystemStatus? _systemStatus;
  SensorData? _sensorData;
  bool _isLoading = true;
  String _errorMessage = '';

  @override
  void initState() {
    super.initState();
    _startMonitoring();
  }

  @override
  void dispose() {
    _updateTimer?.cancel();
    super.dispose();
  }

  void _startMonitoring() {
    _updateData();
    _updateTimer = Timer.periodic(
      Duration(milliseconds: ConfigService.sensors['updateInterval']),
      (timer) => _updateData(),
    );
  }

  Future<void> _updateData() async {
    try {
      final statusFuture = _microcontrollerService.getSystemStatus();
      final sensorFuture = _microcontrollerService.getSensorData();
      
      final results = await Future.wait([statusFuture, sensorFuture]);
      
      if (mounted) {
        setState(() {
          _systemStatus = results[0] as SystemStatus;
          _sensorData = results[1] as SensorData;
          _isLoading = false;
          _errorMessage = '';
        });
      }
    } catch (e) {
      if (mounted) {
        setState(() {
          _errorMessage = 'Error de conexión: $e';
          _isLoading = false;
        });
      }
    }
  }

  Future<void> _toggleSystem() async {
    try {
      bool success;
      if (_systemStatus?.systemActive ?? false) {
        success = await _microcontrollerService.deactivateSystem();
      } else {
        success = await _microcontrollerService.activateSystem();
      }
      
      if (success) {
        _updateData();
      } else {
        _showError('Error cambiando estado del sistema');
      }
    } catch (e) {
      _showError('Error: $e');
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
    if (_isLoading) {
      return const Center(
        child: CircularProgressIndicator(
          color: Color(0xFF00aaff),
        ),
      );
    }

    if (_errorMessage.isNotEmpty) {
      return Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            const Icon(
              Icons.error_outline,
              color: Colors.red,
              size: 48,
            ),
            const SizedBox(height: 16),
            Text(
              _errorMessage,
              style: const TextStyle(color: Colors.red),
              textAlign: TextAlign.center,
            ),
            const SizedBox(height: 16),
            ElevatedButton(
              onPressed: () {
                setState(() {
                  _isLoading = true;
                  _errorMessage = '';
                });
                _updateData();
              },
              child: const Text('Reintentar'),
            ),
          ],
        ),
      );
    }

    return SingleChildScrollView(
      padding: const EdgeInsets.all(16),
      child: Column(
        children: [
          _buildSystemStatusCard(),
          const SizedBox(height: 16),
          _buildSensorDataCard(),
          const SizedBox(height: 16),
          _buildControlsCard(),
        ],
      ),
    );
  }

  Widget _buildSystemStatusCard() {
    final isActive = _systemStatus?.systemActive ?? false;
    return AnimatedCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              const Text(
                '[SYS] Estado del Sistema',
                style: TextStyle(
                  color: Color(0xFF00aaff),
                  fontSize: 18,
                  fontWeight: FontWeight.bold,
                ),
              ),
              PulsingDot(
                color: isActive ? Colors.green : Colors.red,
                size: 12,
                duration: const Duration(milliseconds: 1000),
              ),
            ],
          ),
          const SizedBox(height: 16),
          Row(
            children: [
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      'Estado: ${isActive ? "ACTIVO" : "INACTIVO"}',
                      style: TextStyle(
                        color: isActive ? Colors.green : Colors.red,
                        fontSize: 16,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 8),
                    Text(
                      'Última Clasificación: ${_systemStatus?.lastClassification ?? "Ninguna"}',
                      style: const TextStyle(color: Colors.white70),
                    ),
                    const SizedBox(height: 8),
                    Text(
                      'Motor: ${_systemStatus?.motorStatus ?? "Inactivo"}',
                      style: const TextStyle(color: Colors.white70),
                    ),
                    const SizedBox(height: 8),
                    Text(
                      'Servo: ${_systemStatus?.servoPosition ?? 90}°',
                      style: const TextStyle(color: Colors.white70),
                    ),
                  ],
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildSensorDataCard() {
    return AnimatedCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            '[SNR] Datos de Sensores',
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
                child: Column(
                  children: [
                    AnimatedProgressBar(
                      label: 'Peso Detectado',
                      value: _sensorData?.weightDisplay ?? '0.00 kg',
                      progress: (_sensorData?.weight ?? 0) / 5.0, // Máximo 5kg
                      color: const Color(0xFF4ecdc4),
                      duration: const Duration(milliseconds: 500),
                    ),
                    const SizedBox(height: 16),
                    Container(
                      width: double.infinity,
                      padding: const EdgeInsets.all(12),
                      decoration: BoxDecoration(
                        color: (_sensorData?.pirSensor ?? false)
                            ? Colors.green.withOpacity(0.2)
                            : Colors.red.withOpacity(0.2),
                        border: Border.all(
                          color: (_sensorData?.pirSensor ?? false)
                              ? Colors.green
                              : Colors.red,
                          width: 2,
                        ),
                        borderRadius: BorderRadius.circular(8),
                      ),
                      child: Row(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: [
                          Icon(
                            (_sensorData?.pirSensor ?? false)
                                ? Icons.sensors
                                : Icons.sensors_off,
                            color: (_sensorData?.pirSensor ?? false)
                                ? Colors.green
                                : Colors.red,
                          ),
                          const SizedBox(width: 8),
                          Text(
                            _sensorData?.statusDisplay ?? 'Sin Datos',
                            style: TextStyle(
                              color: (_sensorData?.pirSensor ?? false)
                                  ? Colors.green
                                  : Colors.red,
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                        ],
                      ),
                    ),
                  ],
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildControlsCard() {
    return AnimatedCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            '[CTL] Controles del Sistema',
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
                child: ElevatedButton.icon(
                  onPressed: _toggleSystem,
                  icon: Icon(
                    (_systemStatus?.systemActive ?? false)
                        ? Icons.stop
                        : Icons.play_arrow,
                  ),
                  label: Text(
                    (_systemStatus?.systemActive ?? false)
                        ? 'DESACTIVAR'
                        : 'ACTIVAR',
                  ),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: (_systemStatus?.systemActive ?? false)
                        ? Colors.red
                        : Colors.green,
                    padding: const EdgeInsets.symmetric(vertical: 12),
                  ),
                ),
              ),
            ],
          ),
          const SizedBox(height: 12),
          Row(
            children: [
              Expanded(
                child: ElevatedButton.icon(
                  onPressed: () => _separateMaterial('glass'),
                  icon: const Icon(Icons.recycling),
                  label: const Text('VIDRIO'),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: const Color(0xFF4ecdc4),
                  ),
                ),
              ),
              const SizedBox(width: 8),
              Expanded(
                child: ElevatedButton.icon(
                  onPressed: () => _separateMaterial('plastic'),
                  icon: const Icon(Icons.recycling),
                  label: const Text('PLÁSTICO'),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: const Color(0xFFfeca57),
                  ),
                ),
              ),
              const SizedBox(width: 8),
              Expanded(
                child: ElevatedButton.icon(
                  onPressed: () => _separateMaterial('metal'),
                  icon: const Icon(Icons.recycling),
                  label: const Text('METAL'),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: const Color(0xFFff6b6b),
                  ),
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  Future<void> _separateMaterial(String materialType) async {
    try {
      final success = await _microcontrollerService.separateMaterial(materialType);
      if (success) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Separación de ${ConfigService.getMaterialDisplayName(materialType)} activada'),
            backgroundColor: Colors.green,
          ),
        );
        _updateData();
      } else {
        _showError('Error activando separación de material');
      }
    } catch (e) {
      _showError('Error: $e');
    }
  }
}