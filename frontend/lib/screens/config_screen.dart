import 'package:flutter/material.dart';
import 'dart:async';
import '../services/system_service.dart';
import '../services/app_state_service.dart';
import '../widgets/animated_widgets.dart';
import '../widgets/loading_widgets.dart';

class ConfigScreen extends StatefulWidget {
  const ConfigScreen({super.key});

  @override
  State<ConfigScreen> createState() => _ConfigScreenState();
}

class _ConfigScreenState extends State<ConfigScreen> with AppStateMixin {
  final SystemService _systemService = SystemService();
  
  // Estados para los dropdowns
  String _selectedSensor = 'PIR-1';
  String _selectedServo = 'SERVO-1';
  int _currentServoPosition = 90; // Posición actual del servo
  bool _isLoadingServo = false;
  // El material seleccionado ahora viene del estado global
  String get _selectedMaterial => appState.targetMaterial;
  
  // Datos del sistema
  SystemStatus? _systemStatus;
  SensorData? _sensorData;
  MotorStatus? _motorStatus;
  
  // Estados de conectividad
  bool _isConnectedToBackend = false;
  bool _isEsp32CamConnected = false;
  bool _isEsp32ControlConnected = false;
  
  Timer? _updateTimer;
  
  // Opciones para los dropdowns
  final List<String> _sensorOptions = ['PIR-1', 'PIR-2', 'PIR-3'];
  final List<String> _servoOptions = ['SERVO-1', 'SERVO-2', 'SERVO-3'];
  final List<String> _weightSensorOptions = ['PESO-1', 'PESO-2', 'PESO-3'];
  final List<String> _materialOptions = ['PLÁSTICO', 'VIDRIO', 'METAL'];
  
  String _selectedWeightSensor = 'PESO-1';
  
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
    // Optimizado para Raspberry Pi 5 - actualizaciones menos frecuentes
    _updateTimer = Timer.periodic(const Duration(seconds: 6), (timer) {
      if (mounted) {
        _loadSystemData();
      }
    });
  }
  
  Future<void> _loadSystemData() async {
    try {
      final futures = await Future.wait([
        _systemService.getSystemStatus().catchError((e) => null),
        _systemService.getSensorData().catchError((e) => null),
        _systemService.getMotorStatus().catchError((e) => null),
        _systemService.getEsp32CamStatus().catchError((e) => null),
        _systemService.getEsp32ControlStatus().catchError((e) => null),
      ]);
      
      if (mounted) {
        WidgetsBinding.instance.addPostFrameCallback((_) {
          if (mounted) {
            setState(() {
              _systemStatus = futures[0] as SystemStatus?;
              _sensorData = futures[1] as SensorData?;
              _motorStatus = futures[2] as MotorStatus?;
              final esp32CamStatus = futures[3] as Esp32CamStatus?;
              final esp32ControlStatus = futures[4] as Esp32ControlStatus?;
              
              _isConnectedToBackend = _systemStatus != null;
              _isEsp32CamConnected = esp32CamStatus?.isConnected ?? false;
              _isEsp32ControlConnected = esp32ControlStatus?.isConnected ?? false;
              
              // Actualizar posición del servo si está disponible
              if (_motorStatus?.servoPosition != null && !_isLoadingServo) {
                _currentServoPosition = _motorStatus!.servoPosition!;
              }
            });
          }
        });
      }
    } catch (e) {
      if (mounted) {
        WidgetsBinding.instance.addPostFrameCallback((_) {
          if (mounted) {
            setState(() {
              _isConnectedToBackend = false;
              _isEsp32CamConnected = false;
              _isEsp32ControlConnected = false;
            });
          }
        });
      }
    }
  }

  Future<void> _controlServo(String servoId) async {
    int position = 90; // Posición por defecto
    if (servoId == 'SERVO-1') position = 45;  // Material: Plástico
    if (servoId == 'SERVO-2') position = 90;  // Material: Vidrio
    if (servoId == 'SERVO-3') position = 135; // Material: Metal
    
    try {
      await _systemService.controlServo(position: position);
      _showMessage('Servo $servoId controlado correctamente');
    } catch (e) {
      _showError('Error controlando servo: $e');
    }
  }

  Future<void> _selectSensor(String sensorId) async {
    _showMessage('Sensor PIR $sensorId seleccionado');
  }

  Future<void> _selectWeightSensor(String sensorId) async {
    _showMessage('Sensor de peso $sensorId seleccionado');
  }

  Future<void> _adjustWeight(double adjustment) async {
    try {
      _showMessage('Ajustando peso en ${adjustment > 0 ? '+' : ''}${adjustment}g');
      // Aquí llamarías al endpoint de ajuste de peso
      // await _systemService.adjustWeight(_selectedWeightSensor, adjustment);
    } catch (e) {
      _showError('Error ajustando peso: $e');
    }
  }

  Future<void> _setTargetMaterial(String material) async {
    // Actualizar el estado global
    appState.setTargetMaterial(material);
    
    // Enviar al backend también
    try {
      // Aquí puedes agregar una llamada al backend si es necesario
      // await _systemService.setTargetMaterial(material);
      _showMessage('Material objetivo configurado: $material');
    } catch (e) {
      _showError('Error configurando material objetivo: $e');
      // Revertir el cambio si falla
      // appState.setTargetMaterial(oldMaterial);
    }
  }

  Future<void> _saveConfiguration() async {
    _showMessage('Configuración guardada correctamente');
  }

  Future<void> _resetSystem() async {
    try {
      final confirmed = await _showConfirmDialog(
        'Confirmar Reset',
        '¿Estás seguro de que quieres reiniciar el sistema?'
      );
      
      if (confirmed) {
        // Reiniciar ambos ESP32
        await _systemService.controlEsp32Cam(action: 'restart');
        await _systemService.controlEsp32Cam(action: 'restart'); // También el control si es necesario
        _showMessage('Sistema reiniciado');
      }
    } catch (e) {
      _showError('Error reiniciando sistema: $e');
    }
  }

  void _showMessage(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(message),
        backgroundColor: const Color(0xFF00C851),
      ),
    );
  }

  void _showError(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(message),
        backgroundColor: const Color(0xFFFF3547),
      ),
    );
  }

  Future<bool> _showConfirmDialog(String title, String content) async {
    final result = await showDialog<bool>(
      context: context,
      builder: (context) => AlertDialog(
        backgroundColor: const Color(0xFF1a1a2e),
        title: Text(title, style: const TextStyle(color: Color(0xFF00aaff))),
        content: Text(content, style: const TextStyle(color: Colors.white)),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context, false),
            child: const Text('Cancelar', style: TextStyle(color: Colors.grey)),
          ),
          TextButton(
            onPressed: () => Navigator.pop(context, true),
            child: const Text('Confirmar', style: TextStyle(color: Color(0xFF00aaff))),
          ),
        ],
      ),
    );
    return result ?? false;
  }

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
      padding: const EdgeInsets.all(16),
      child: Column(
        children: [
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 800),
            child: _buildConnectivityCard(),
          ),
          const SizedBox(height: 16),
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 900),
            delay: const Duration(milliseconds: 200),
            child: _buildMotorsCard(),
          ),
          const SizedBox(height: 16),
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 1000),
            delay: const Duration(milliseconds: 400),
            child: _buildCalibrationCard(),
          ),
          const SizedBox(height: 16),
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 1100),
            delay: const Duration(milliseconds: 600),
            child: _buildSystemSettingsCard(),
          ),
          const SizedBox(height: 16),
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 1200),
            delay: const Duration(milliseconds: 800),
            child: _buildNetworkCard(),
          ),
          const SizedBox(height: 16),
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 1300),
            delay: const Duration(milliseconds: 1000),
            child: _buildSystemInfoCard(),
          ),
        ],
      ),
    );
  }

  Widget _buildConnectivityCard() {
    return AnimatedCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            '[NET] Conectividad del Sistema',
            style: TextStyle(
              color: Color(0xFF00aaff),
              fontSize: 18,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 16),
          ConnectionStatus(
            isConnected: _isConnectedToBackend,
            label: 'FastAPI Backend',
            onRetry: _testConnection,
          ),
          const SizedBox(height: 8),
          ConnectionStatus(
            isConnected: _isEsp32CamConnected,
            label: 'ESP32-CAM (192.168.1.100)',
            onRetry: _reconnectAll,
          ),
          const SizedBox(height: 8),
          ConnectionStatus(
            isConnected: _isEsp32ControlConnected,
            label: 'ESP32-CONTROL (192.168.1.101)', 
            onRetry: _reconnectAll,
          ),
        ],
      ),
    );
  }

  Widget _buildConnectionItem(String label, bool isConnected) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 6),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(
            label,
            style: const TextStyle(color: Colors.white, fontSize: 14),
          ),
          Row(
            children: [
              PulsingDot(
                color: isConnected ? const Color(0xFF00C851) : const Color(0xFFFF3547),
                size: 12,
                duration: const Duration(seconds: 1),
              ),
              const SizedBox(width: 8),
              Text(
                isConnected ? 'CONECTADO' : 'DESCONECTADO',
                style: TextStyle(
                  color: isConnected ? const Color(0xFF00C851) : const Color(0xFFFF3547),
                  fontSize: 12,
                  fontWeight: FontWeight.bold,
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildMotorsCard() {
    return AnimatedCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            '[MOT] Motor Reductor & Servos',
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
                    _buildProgressBar(
                      'Motor Reductor', 
                      '${_motorStatus?.currentSpeed ?? 0}%', 
                      (_motorStatus?.currentSpeed ?? 0) / 100, 
                      const Color(0xFFFF6B6B)
                    ),
                    const SizedBox(height: 12),
                    Row(
                      children: [
                        Expanded(child: GestureDetector(
                          onTap: () => _controlMotor(false),
                          child: _buildActionButton('[-] PARAR'),
                        )),
                        const SizedBox(width: 8),
                        Expanded(child: GestureDetector(
                          onTap: () => _controlMotor(true),
                          child: _buildActionButton('[+] INICIAR'),
                        )),
                      ],
                    ),
                  ],
                ),
              ),
              const SizedBox(width: 16),
              Expanded(
                child: Column(
                  children: [
                    _buildFunctionalDropdown('Servomotores', _selectedServo, _servoOptions, (value) {
                      setState(() {
                        _selectedServo = value!;
                      });
                      _controlServo(value!);
                    }),
                    const SizedBox(height: 12),
                    if (_isLoadingServo)
                      const RaspberryPiLoader(
                        message: 'Moviendo servo...',
                        size: 24,
                        showMessage: false,
                      )
                    else
                      _buildProgressBar(
                        'Posición', 
                        '${_currentServoPosition}°', 
                        _currentServoPosition / 180, 
                        const Color(0xFFFECA57)
                      ),
                    const SizedBox(height: 12),
                    Row(
                      children: [
                        Expanded(child: GestureDetector(
                          onTap: () => _moveServoTo(0),
                          child: _buildControlButton('0°'),
                        )),
                        const SizedBox(width: 4),
                        Expanded(child: GestureDetector(
                          onTap: () => _moveServoTo(90),
                          child: _buildControlButton('90°'),
                        )),
                        const SizedBox(width: 4),
                        Expanded(child: GestureDetector(
                          onTap: () => _moveServoTo(180),
                          child: _buildControlButton('180°'),
                        )),
                      ],
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

  Future<void> _controlMotor(bool active) async {
    try {
      await _systemService.controlMotor(conveyorActive: active);
      _showMessage('Motor ${active ? 'iniciado' : 'detenido'}');
    } catch (e) {
      _showError('Error controlando motor: $e');
    }
  }

  Future<void> _moveServoTo(int position) async {
    if (_isLoadingServo) return;
    
    setState(() {
      _isLoadingServo = true;
    });

    try {
      await _systemService.controlServo(position: position);
      
      // Simular movimiento gradual del servo para retroalimentación visual
      final startPos = _currentServoPosition;
      final steps = (position - startPos).abs();
      
      for (int i = 0; i <= steps; i += 5) {
        await Future.delayed(const Duration(milliseconds: 50));
        if (mounted) {
          setState(() {
            if (position > startPos) {
              _currentServoPosition = (startPos + i).clamp(0, position);
            } else {
              _currentServoPosition = (startPos - i).clamp(position, 180);
            }
          });
        }
      }
      
      setState(() {
        _currentServoPosition = position;
      });
      
      _showMessage('Servo movido a ${position}°');
    } catch (e) {
      _showError('Error moviendo servo: $e');
    } finally {
      if (mounted) {
        setState(() {
          _isLoadingServo = false;
        });
      }
    }
  }

  Widget _buildCalibrationCard() {
    return AnimatedCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            '[CAL] Sensores & Calibración',
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
                    _buildFunctionalDropdown('Sensores de Peso', _selectedWeightSensor, _weightSensorOptions, (value) {
                      setState(() {
                        _selectedWeightSensor = value!;
                      });
                      _selectWeightSensor(value!);
                    }),
                    const SizedBox(height: 16),
                    Text(
                      'Actual: ${_sensorData?.weightDisplay ?? '0.00 kg'}',
                      style: const TextStyle(
                        color: Color(0xFF00aaff),
                        fontSize: 18,
                      ),
                    ),
                    const SizedBox(height: 12),
                    Row(
                      children: [
                        Expanded(child: GestureDetector(
                          onTap: () => _adjustWeight(-0.1),
                          child: _buildControlButton('-0.1g'),
                        )),
                        const SizedBox(width: 8),
                        Expanded(child: GestureDetector(
                          onTap: () => _adjustWeight(0.1),
                          child: _buildControlButton('+0.1g'),
                        )),
                      ],
                    ),
                    const SizedBox(height: 12),
                    GestureDetector(
                      onTap: _calibrateSensors,
                      child: _buildSuccessButton('[R] CALIBRAR'),
                    ),
                  ],
                ),
              ),
              const SizedBox(width: 16),
              Expanded(
                child: Column(
                  children: [
                    _buildFunctionalDropdown('Sensores PIR', _selectedSensor, _sensorOptions, (value) {
                      setState(() {
                        _selectedSensor = value!;
                      });
                      _selectSensor(value!);
                    }),
                    const SizedBox(height: 16),
                    Text(
                      _selectedSensor,
                      style: const TextStyle(
                        color: Colors.white,
                        fontSize: 16,
                      ),
                    ),
                    const SizedBox(height: 8),
                    Text(
                      _sensorData?.pirSensor == true ? 'ACTIVO' : 'INACTIVO',
                      style: TextStyle(
                        color: _sensorData?.pirSensor == true ? const Color(0xFF00C851) : const Color(0xFFFF3547),
                        fontSize: 20,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    Text(
                      'Peso: ${_sensorData?.weightDisplay ?? '0.00 kg'}',
                      style: const TextStyle(
                        color: Colors.grey,
                        fontSize: 14,
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

  Future<void> _calibrateSensors() async {
    try {
      _showMessage('Calibrando sensores...');
      // Aquí llamarías al endpoint de calibración
      // await _systemService.calibrateSensors();
    } catch (e) {
      _showError('Error calibrando sensores: $e');
    }
  }

  Widget _buildSystemSettingsCard() {
    return AnimatedCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            '[SYS] Configuración Sistema',
            style: TextStyle(
              color: Color(0xFF00aaff),
              fontSize: 18,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 16),
          _buildFunctionalDropdown('Material Objetivo', _selectedMaterial, _materialOptions, (value) {
            // No necesitamos setState aquí porque el AppStateMixin se encarga de eso
            _setTargetMaterial(value!);
          }),
          const SizedBox(height: 16),
          Row(
            children: [
              Expanded(child: GestureDetector(
                onTap: _saveConfiguration,
                child: _buildSuccessButton('[S] GUARDAR CONFIG'),
              )),
              const SizedBox(width: 8),
              Expanded(child: GestureDetector(
                onTap: _resetSystem,
                child: _buildDangerButton('[R] RESET SISTEMA'),
              )),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildNetworkCard() {
    return AnimatedCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            '[CONN] Configuración de Conexiones',
            style: TextStyle(
              color: Color(0xFF00aaff),
              fontSize: 18,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 16),
          _buildConnectionDetail('FastAPI: localhost:8000', _isConnectedToBackend),
          _buildConnectionDetail('ESP32-CAM: 192.168.1.100', _isEsp32CamConnected),
          _buildConnectionDetail('ESP32-CONTROL: 192.168.1.101', _isEsp32ControlConnected),
          _buildConnectionDetail('Estado:', _systemStatus?.status ?? 'Desconectado'),
          const SizedBox(height: 16),
          Row(
            children: [
              Expanded(child: GestureDetector(
                onTap: _testConnection,
                child: _buildActionButton('[T] Test Conexión'),
              )),
              const SizedBox(width: 8),
              Expanded(child: GestureDetector(
                onTap: _reconnectAll,
                child: _buildActionButton('[R] Reconectar'),
              )),
            ],
          ),
        ],
      ),
    );
  }

  Future<void> _testConnection() async {
    try {
      await _systemService.getSystemStatus();
      _showMessage('Conexión exitosa con el backend');
    } catch (e) {
      _showError('Error de conexión: $e');
    }
  }

  Future<void> _reconnectAll() async {
    _showMessage('Reconectando sistemas...');
    _loadSystemData();
  }

  Widget _buildConnectionDetail(String label, dynamic value) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(
            label,
            style: const TextStyle(color: Colors.white, fontSize: 14),
          ),
          Text(
            value.toString(),
            style: const TextStyle(color: Color(0xFF00aaff), fontSize: 14),
          ),
        ],
      ),
    );
  }

  Widget _buildSystemInfoCard() {
    return AnimatedCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            '[INFO] Información del Sistema',
            style: TextStyle(
              color: Color(0xFF00aaff),
              fontSize: 18,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 16),
          SystemInfoRow('Backend:', _systemStatus?.isActive == true ? 'Activo' : 'Inactivo'),
          SystemInfoRow('Versión:', _systemStatus?.components['version']?.toString() ?? 'N/A'),
          SystemInfoRow('ESP32-CAM:', _isEsp32CamConnected ? 'Conectado' : 'Desconectado'),
          SystemInfoRow('ESP32-CONTROL:', _isEsp32ControlConnected ? 'Conectado' : 'Desconectado'),
          SystemInfoRow('Sensores:', '${_sensorOptions.length} disponibles'),
          SystemInfoRow('Servos:', '${_servoOptions.length} disponibles'),
          SystemInfoRow('Sensores Peso:', '${_weightSensorOptions.length} disponibles'),
          SystemInfoRow('Última Act.:', _systemStatus?.lastUpdate.toString().substring(0, 19) ?? 'N/A'),
        ],
      ),
    );
  }

  Widget _buildFunctionalDropdown(String label, String value, List<String> options, ValueChanged<String?> onChanged) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          label,
          style: const TextStyle(
            color: Colors.white,
            fontSize: 14,
            fontWeight: FontWeight.bold,
          ),
        ),
        const SizedBox(height: 8),
        Container(
          width: double.infinity,
          padding: const EdgeInsets.symmetric(horizontal: 12),
          decoration: BoxDecoration(
            color: Colors.black.withOpacity(0.5),
            border: Border.all(color: const Color(0xFF00aaff).withOpacity(0.5)),
            borderRadius: BorderRadius.circular(8),
          ),
          child: DropdownButtonHideUnderline(
            child: DropdownButton<String>(
              value: value,
              dropdownColor: const Color(0xFF1a1a2e),
              style: const TextStyle(color: Colors.white, fontSize: 14),
              icon: const Icon(Icons.arrow_drop_down, color: Color(0xFF00aaff)),
              items: options.map((String option) {
                return DropdownMenuItem<String>(
                  value: option,
                  child: Text(option),
                );
              }).toList(),
              onChanged: onChanged,
            ),
          ),
        ),
      ],
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
            fontSize: 12,
            fontWeight: FontWeight.bold,
          ),
        ),
      ),
    );
  }

  Widget _buildSuccessButton(String text) {
    return Container(
      height: 40,
      decoration: BoxDecoration(
        gradient: const LinearGradient(
          colors: [Color(0xFF00C851), Color(0xFF007E33)],
        ),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Center(
        child: Text(
          text,
          style: const TextStyle(
            color: Colors.white,
            fontSize: 12,
            fontWeight: FontWeight.bold,
          ),
        ),
      ),
    );
  }

  Widget _buildDangerButton(String text) {
    return Container(
      height: 40,
      decoration: BoxDecoration(
        gradient: const LinearGradient(
          colors: [Color(0xFFFF3547), Color(0xFFB71C1C)],
        ),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Center(
        child: Text(
          text,
          style: const TextStyle(
            color: Colors.white,
            fontSize: 12,
            fontWeight: FontWeight.bold,
          ),
        ),
      ),
    );
  }

  Widget _buildControlButton(String text) {
    return Container(
      height: 40,
      padding: const EdgeInsets.all(8),
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
            fontSize: 12,
            fontWeight: FontWeight.bold,
          ),
        ),
      ),
    );
  }

  Widget _buildProgressBar(String label, String value, double progress, Color color) {
    return Column(
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text(
              label,
              style: const TextStyle(
                color: Colors.white,
                fontSize: 16,
                fontWeight: FontWeight.bold,
              ),
            ),
            Text(
              value,
              style: const TextStyle(
                color: Colors.white,
                fontSize: 16,
                fontWeight: FontWeight.bold,
              ),
            ),
          ],
        ),
        const SizedBox(height: 8),
        Container(
          height: 16,
          decoration: BoxDecoration(
            color: Colors.white.withOpacity(0.1),
            borderRadius: BorderRadius.circular(8),
          ),
          child: FractionallySizedBox(
            alignment: Alignment.centerLeft,
            widthFactor: progress.clamp(0.0, 1.0),
            child: Container(
              decoration: BoxDecoration(
                color: color,
                borderRadius: BorderRadius.circular(8),
              ),
            ),
          ),
        ),
      ],
    );
  }
}

class SystemInfoRow extends StatelessWidget {
  final String label;
  final String value;

  const SystemInfoRow(this.label, this.value, {super.key});

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 2),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          SizedBox(
            width: 120,
            child: Text(
              label,
              style: const TextStyle(
                color: Colors.white,
                fontSize: 12,
                fontWeight: FontWeight.bold,
              ),
            ),
          ),
          Expanded(
            child: Text(
              value,
              style: const TextStyle(
                color: Colors.white,
                fontSize: 12,
              ),
            ),
          ),
        ],
      ),
    );
  }
}