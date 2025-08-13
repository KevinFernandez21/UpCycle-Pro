import 'package:flutter/material.dart';
import '../widgets/animated_widgets.dart';

/// Ejemplos avanzados de combinaciones de animaciones
/// Casos de uso reales del sistema UpCycle Pro
class AdvancedAnimationExamples extends StatefulWidget {
  const AdvancedAnimationExamples({super.key});

  @override
  State<AdvancedAnimationExamples> createState() => _AdvancedAnimationExamplesState();
}

class _AdvancedAnimationExamplesState extends State<AdvancedAnimationExamples> {
  bool _isSystemOnline = true;
  double _cpuUsage = 0.65;
  int _detectedObjects = 3;
  
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFF1a1a2e),
      appBar: AppBar(
        title: const Text('Ejemplos Avanzados'),
        backgroundColor: Colors.black.withOpacity(0.3),
        foregroundColor: Colors.white,
      ),
      body: Container(
        decoration: const BoxDecoration(
          gradient: LinearGradient(
            colors: [Color(0xFF1a1a2e), Color(0xFF16213e), Color(0xFF0f3460)],
            begin: Alignment.topLeft,
            end: Alignment.bottomRight,
          ),
        ),
        child: SingleChildScrollView(
          padding: const EdgeInsets.all(16),
          child: Column(
            children: [
              _buildSystemStatusExample(),
              const SizedBox(height: 24),
              _buildCameraControlExample(),
              const SizedBox(height: 24),
              _buildSensorGridExample(),
              const SizedBox(height: 24),
              _buildDetectionExample(),
              const SizedBox(height: 24),
              _buildControlButtonsExample(),
            ],
          ),
        ),
      ),
    );
  }

  /// EJEMPLO 1: Sistema de Estado Completo
  Widget _buildSystemStatusExample() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text(
          '🖥️ Sistema de Estado Completo',
          style: TextStyle(
            color: Color(0xFF00aaff),
            fontSize: 20,
            fontWeight: FontWeight.bold,
          ),
        ),
        const SizedBox(height: 16),
        
        SlideInAnimation(
          begin: const Offset(0, -0.3),
          duration: const Duration(milliseconds: 800),
          child: AnimatedCard(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                // Header del sistema
                Row(
                  children: [
                    PulsingDot(
                      color: _isSystemOnline ? Colors.green : Colors.red,
                      size: 16,
                      duration: _isSystemOnline 
                          ? const Duration(seconds: 2) 
                          : const Duration(milliseconds: 500),
                    ),
                    const SizedBox(width: 12),
                    Text(
                      _isSystemOnline ? 'Sistema Online' : 'Sistema Offline',
                      style: const TextStyle(
                        color: Colors.white,
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const Spacer(),
                    Switch(
                      value: _isSystemOnline,
                      onChanged: (value) {
                        setState(() {
                          _isSystemOnline = value;
                        });
                      },
                    ),
                  ],
                ),
                const SizedBox(height: 20),
                
                // Progreso del sistema
                AnimatedProgressBar(
                  label: 'CPU Usage',
                  value: '${(_cpuUsage * 100).toInt()}%',
                  progress: _cpuUsage,
                  color: _cpuUsage > 0.8 ? Colors.red : 
                         _cpuUsage > 0.6 ? Colors.orange : Colors.green,
                  duration: const Duration(milliseconds: 1500),
                ),
                const SizedBox(height: 16),
                
                AnimatedProgressBar(
                  label: 'Memory',
                  value: '7.2 GB',
                  progress: 0.72,
                  color: const Color(0xFF4ecdc4),
                  duration: const Duration(milliseconds: 2000),
                ),
                const SizedBox(height: 20),
                
                // Botones de control
                Row(
                  children: [
                    Expanded(
                      child: FloatingButton(
                        duration: const Duration(seconds: 3),
                        onPressed: () {
                          setState(() {
                            _cpuUsage = _cpuUsage == 0.65 ? 0.35 : 0.65;
                          });
                        },
                        child: Container(
                          height: 40,
                          decoration: BoxDecoration(
                            gradient: const LinearGradient(
                              colors: [Color(0xFF667eea), Color(0xFF764ba2)],
                            ),
                            borderRadius: BorderRadius.circular(8),
                          ),
                          child: const Center(
                            child: Text(
                              'Toggle CPU',
                              style: TextStyle(color: Colors.white),
                            ),
                          ),
                        ),
                      ),
                    ),
                    const SizedBox(width: 12),
                    Expanded(
                      child: FloatingButton(
                        duration: const Duration(seconds: 4),
                        child: Container(
                          height: 40,
                          decoration: BoxDecoration(
                            gradient: const LinearGradient(
                              colors: [Color(0xFF4ecdc4), Color(0xFF44a08d)],
                            ),
                            borderRadius: BorderRadius.circular(8),
                          ),
                          child: const Center(
                            child: Text(
                              'Refresh',
                              style: TextStyle(color: Colors.white),
                            ),
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
        
        const SizedBox(height: 16),
        _buildCodeExample('''
// Ejemplo completo de sistema con estado
SlideInAnimation(
  begin: Offset(0, -0.3),
  child: AnimatedCard(
    child: Column(
      children: [
        // Indicador de estado con pulso
        PulsingDot(
          color: isOnline ? Colors.green : Colors.red,
          duration: isOnline ? Duration(seconds: 2) : Duration(milliseconds: 500),
        ),
        
        // Barra de progreso reactiva
        AnimatedProgressBar(
          label: 'CPU Usage',
          value: '\${cpuUsage.toInt()}%',
          progress: cpuUsage,
          color: cpuUsage > 0.8 ? Colors.red : Colors.green,
        ),
        
        // Botones flotantes
        FloatingButton(
          child: MyButton(),
          onPressed: () => updateCPU(),
        ),
      ],
    ),
  ),
)'''),
      ],
    );
  }

  /// EJEMPLO 2: Control de Cámara Avanzado
  Widget _buildCameraControlExample() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text(
          '📹 Control de Cámara Avanzado',
          style: TextStyle(
            color: Color(0xFF00aaff),
            fontSize: 20,
            fontWeight: FontWeight.bold,
          ),
        ),
        const SizedBox(height: 16),
        
        SlideInAnimation(
          begin: const Offset(-0.3, 0),
          duration: const Duration(milliseconds: 1000),
          child: AnimatedCard(
            child: Column(
              children: [
                // Status de cámara en vivo
                Row(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    const PulsingDot(
                      color: Colors.green,
                      size: 12,
                      duration: Duration(milliseconds: 800),
                    ),
                    const SizedBox(width: 12),
                    const Text(
                      'EN VIVO - 1080p',
                      style: TextStyle(
                        color: Colors.green,
                        fontSize: 16,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                  ],
                ),
                const SizedBox(height: 20),
                
                // Vista previa de cámara
                Container(
                  width: double.infinity,
                  height: 120,
                  decoration: BoxDecoration(
                    gradient: const LinearGradient(
                      colors: [Color(0xFF1e3c72), Color(0xFF2a5298)],
                    ),
                    borderRadius: BorderRadius.circular(8),
                    border: Border.all(color: Colors.white.withOpacity(0.2)),
                  ),
                  child: const Center(
                    child: Column(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: [
                        RotatingIcon(
                          icon: Icons.videocam,
                          color: Colors.white70,
                          size: 32,
                          duration: Duration(seconds: 6),
                        ),
                        SizedBox(height: 8),
                        Text(
                          'Camera Active',
                          style: TextStyle(color: Colors.white70),
                        ),
                      ],
                    ),
                  ),
                ),
                const SizedBox(height: 20),
                
                // Controles de cámara
                Row(
                  children: [
                    Expanded(
                      child: FloatingButton(
                        duration: const Duration(seconds: 3),
                        child: _buildCameraButton('[+] ZOOM'),
                      ),
                    ),
                    const SizedBox(width: 8),
                    Expanded(
                      child: FloatingButton(
                        duration: const Duration(seconds: 4),
                        child: _buildCameraButton('[-] ZOOM'),
                      ),
                    ),
                    const SizedBox(width: 8),
                    Expanded(
                      child: FloatingButton(
                        duration: const Duration(seconds: 2),
                        child: _buildCameraButton('[*] FOTO'),
                      ),
                    ),
                  ],
                ),
                const SizedBox(height: 16),
                
                // Configuración de cámara
                AnimatedProgressBar(
                  label: 'Focus',
                  value: '87%',
                  progress: 0.87,
                  color: const Color(0xFF4ecdc4),
                  duration: const Duration(milliseconds: 2000),
                ),
              ],
            ),
          ),
        ),
        
        const SizedBox(height: 16),
        _buildCodeExample('''
// Control de cámara con múltiples animaciones
AnimatedCard(
  child: Column(
    children: [
      // Indicador de transmisión en vivo
      Row(
        children: [
          PulsingDot(
            color: Colors.green,
            duration: Duration(milliseconds: 800),
          ),
          Text('EN VIVO - 1080p'),
        ],
      ),
      
      // Vista con icono rotatorio
      RotatingIcon(
        icon: Icons.videocam,
        duration: Duration(seconds: 6),
      ),
      
      // Controles flotantes
      Row(
        children: [
          FloatingButton(
            duration: Duration(seconds: 3),
            child: CameraButton('[+] ZOOM'),
          ),
          FloatingButton(
            duration: Duration(seconds: 4),
            child: CameraButton('[-] ZOOM'),
          ),
        ],
      ),
      
      // Configuración con progreso
      AnimatedProgressBar(
        label: 'Focus',
        progress: 0.87,
        color: Color(0xFF4ecdc4),
      ),
    ],
  ),
)'''),
      ],
    );
  }

  /// EJEMPLO 3: Grid de Sensores
  Widget _buildSensorGridExample() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text(
          '🔍 Grid de Sensores PIR',
          style: TextStyle(
            color: Color(0xFF00aaff),
            fontSize: 20,
            fontWeight: FontWeight.bold,
          ),
        ),
        const SizedBox(height: 16),
        
        SlideInAnimation(
          begin: const Offset(0.3, 0),
          duration: const Duration(milliseconds: 1200),
          child: AnimatedCard(
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
                
                // Grid de sensores con animaciones escalonadas
                Row(
                  children: [
                    Expanded(
                      child: SlideInAnimation(
                        begin: const Offset(-0.3, 0),
                        duration: const Duration(milliseconds: 600),
                        delay: const Duration(milliseconds: 200),
                        child: _buildSensorWidget('PIR-1', true),
                      ),
                    ),
                    Expanded(
                      child: SlideInAnimation(
                        begin: const Offset(-0.1, 0),
                        duration: const Duration(milliseconds: 600),
                        delay: const Duration(milliseconds: 400),
                        child: _buildSensorWidget('PIR-2', true),
                      ),
                    ),
                    Expanded(
                      child: SlideInAnimation(
                        begin: const Offset(0.1, 0),
                        duration: const Duration(milliseconds: 600),
                        delay: const Duration(milliseconds: 600),
                        child: _buildSensorWidget('PIR-3', false),
                      ),
                    ),
                    Expanded(
                      child: SlideInAnimation(
                        begin: const Offset(0.3, 0),
                        duration: const Duration(milliseconds: 600),
                        delay: const Duration(milliseconds: 800),
                        child: _buildSensorWidget('PIR-4', true),
                      ),
                    ),
                  ],
                ),
              ],
            ),
          ),
        ),
        
        const SizedBox(height: 16),
        _buildCodeExample('''
// Grid de sensores con animaciones escalonadas
Row(
  children: [
    SlideInAnimation(
      begin: Offset(-0.3, 0),
      delay: Duration(milliseconds: 200),
      child: SensorWidget('PIR-1', true),
    ),
    SlideInAnimation(
      begin: Offset(-0.1, 0),
      delay: Duration(milliseconds: 400),
      child: SensorWidget('PIR-2', true),
    ),
    SlideInAnimation(
      begin: Offset(0.1, 0),
      delay: Duration(milliseconds: 600),
      child: SensorWidget('PIR-3', false),
    ),
    SlideInAnimation(
      begin: Offset(0.3, 0),
      delay: Duration(milliseconds: 800),
      child: SensorWidget('PIR-4', true),
    ),
  ],
)'''),
      ],
    );
  }

  /// EJEMPLO 4: Sistema de Detección Inteligente
  Widget _buildDetectionExample() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text(
          '🤖 Sistema de Detección IA',
          style: TextStyle(
            color: Color(0xFF00aaff),
            fontSize: 20,
            fontWeight: FontWeight.bold,
          ),
        ),
        const SizedBox(height: 16),
        
        SlideInAnimation(
          begin: const Offset(0, 0.3),
          duration: const Duration(milliseconds: 1000),
          child: AnimatedCard(
            onTap: () {
              setState(() {
                _detectedObjects = _detectedObjects == 3 ? 7 : 3;
              });
            },
            child: Column(
              children: [
                const Text(
                  '[AI] Detección Activa',
                  style: TextStyle(
                    color: Color(0xFF00aaff),
                    fontSize: 18,
                    fontWeight: FontWeight.bold,
                  ),
                ),
                const SizedBox(height: 20),
                
                // Indicador de proceso activo
                Row(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    const PulsingDot(
                      color: Color(0xFF00aaff),
                      size: 16,
                      duration: Duration(seconds: 1),
                    ),
                    const SizedBox(width: 16),
                    const RotatingIcon(
                      icon: Icons.psychology,
                      color: Color(0xFF00aaff),
                      size: 24,
                      duration: Duration(seconds: 3),
                    ),
                    const SizedBox(width: 16),
                    const Text(
                      'Procesando...',
                      style: TextStyle(
                        color: Colors.white,
                        fontSize: 16,
                      ),
                    ),
                  ],
                ),
                const SizedBox(height: 20),
                
                // Resultados animados
                SlideInAnimation(
                  key: ValueKey(_detectedObjects),
                  begin: const Offset(0, 0.3),
                  duration: const Duration(milliseconds: 800),
                  child: Text(
                    'Objetos detectados: $_detectedObjects',
                    style: const TextStyle(
                      color: Color(0xFF00aaff),
                      fontSize: 24,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                ),
                const SizedBox(height: 12),
                
                // Barra de confianza
                AnimatedProgressBar(
                  label: 'Confianza',
                  value: '95.7%',
                  progress: 0.957,
                  color: Colors.green,
                  duration: const Duration(milliseconds: 1500),
                ),
                const SizedBox(height: 16),
                
                const Text(
                  'Toca para cambiar detección',
                  style: TextStyle(
                    color: Colors.grey,
                    fontSize: 12,
                  ),
                ),
              ],
            ),
          ),
        ),
        
        const SizedBox(height: 16),
        _buildCodeExample('''
// Sistema de detección con IA
AnimatedCard(
  onTap: () => updateDetection(),
  child: Column(
    children: [
      // Procesamiento activo
      Row(
        children: [
          PulsingDot(
            color: Color(0xFF00aaff),
            duration: Duration(seconds: 1),
          ),
          RotatingIcon(
            icon: Icons.psychology,
            duration: Duration(seconds: 3),
          ),
          Text('Procesando...'),
        ],
      ),
      
      // Resultados con animación de cambio
      SlideInAnimation(
        key: ValueKey(detectedObjects),
        child: Text('Objetos: \$detectedObjects'),
      ),
      
      // Nivel de confianza
      AnimatedProgressBar(
        label: 'Confianza',
        progress: 0.957,
        color: Colors.green,
      ),
    ],
  ),
)'''),
      ],
    );
  }

  /// EJEMPLO 5: Botones de Control Avanzados
  Widget _buildControlButtonsExample() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text(
          '🎮 Botones de Control Avanzados',
          style: TextStyle(
            color: Color(0xFF00aaff),
            fontSize: 20,
            fontWeight: FontWeight.bold,
          ),
        ),
        const SizedBox(height: 16),
        
        SlideInAnimation(
          begin: const Offset(0, 0.5),
          duration: const Duration(milliseconds: 1000),
          child: AnimatedCard(
            child: Column(
              children: [
                const Text(
                  'Controles del Sistema',
                  style: TextStyle(
                    color: Colors.white,
                    fontSize: 18,
                    fontWeight: FontWeight.bold,
                  ),
                ),
                const SizedBox(height: 20),
                
                // Fila de botones de acción
                Row(
                  children: [
                    Expanded(
                      child: FloatingButton(
                        duration: const Duration(seconds: 3),
                        onPressed: () {
                          ScaffoldMessenger.of(context).showSnackBar(
                            const SnackBar(content: Text('Sistema iniciado!')),
                          );
                        },
                        child: _buildActionButton(
                          'INICIAR',
                          const Color(0xFF4ecdc4),
                          Icons.play_arrow,
                        ),
                      ),
                    ),
                    const SizedBox(width: 12),
                    Expanded(
                      child: FloatingButton(
                        duration: const Duration(seconds: 4),
                        onPressed: () {
                          ScaffoldMessenger.of(context).showSnackBar(
                            const SnackBar(content: Text('Sistema pausado!')),
                          );
                        },
                        child: _buildActionButton(
                          'PAUSAR',
                          Colors.orange,
                          Icons.pause,
                        ),
                      ),
                    ),
                    const SizedBox(width: 12),
                    Expanded(
                      child: FloatingButton(
                        duration: const Duration(seconds: 2),
                        onPressed: () {
                          ScaffoldMessenger.of(context).showSnackBar(
                            const SnackBar(content: Text('Sistema detenido!')),
                          );
                        },
                        child: _buildActionButton(
                          'DETENER',
                          Colors.red,
                          Icons.stop,
                        ),
                      ),
                    ),
                  ],
                ),
                const SizedBox(height: 20),
                
                // Botón de emergencia especial
                FloatingButton(
                  duration: const Duration(seconds: 1),
                  child: Container(
                    width: double.infinity,
                    height: 60,
                    decoration: BoxDecoration(
                      gradient: const LinearGradient(
                        colors: [Colors.red, Colors.redAccent],
                      ),
                      borderRadius: BorderRadius.circular(12),
                      boxShadow: [
                        BoxShadow(
                          color: Colors.red.withOpacity(0.4),
                          blurRadius: 8,
                          spreadRadius: 2,
                        ),
                      ],
                    ),
                    child: const Row(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: [
                        Icon(Icons.emergency, color: Colors.white, size: 24),
                        SizedBox(width: 12),
                        Text(
                          'PARADA DE EMERGENCIA',
                          style: TextStyle(
                            color: Colors.white,
                            fontSize: 16,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                      ],
                    ),
                  ),
                ),
              ],
            ),
          ),
        ),
        
        const SizedBox(height: 16),
        _buildCodeExample('''
// Botones de control con diferentes animaciones
Row(
  children: [
    FloatingButton(
      duration: Duration(seconds: 3),
      child: ActionButton('INICIAR', Colors.green, Icons.play_arrow),
    ),
    FloatingButton(
      duration: Duration(seconds: 4),
      child: ActionButton('PAUSAR', Colors.orange, Icons.pause),
    ),
    FloatingButton(
      duration: Duration(seconds: 2),
      child: ActionButton('DETENER', Colors.red, Icons.stop),
    ),
  ],
)

// Botón de emergencia especial
FloatingButton(
  duration: Duration(seconds: 1),
  child: Container(
    decoration: BoxDecoration(
      gradient: LinearGradient(colors: [Colors.red, Colors.redAccent]),
      boxShadow: [BoxShadow(color: Colors.red.withOpacity(0.4))],
    ),
    child: Row(
      children: [
        Icon(Icons.emergency),
        Text('PARADA DE EMERGENCIA'),
      ],
    ),
  ),
)'''),
      ],
    );
  }

  // Widgets auxiliares
  Widget _buildCameraButton(String text) {
    return Container(
      height: 45,
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

  Widget _buildSensorWidget(String label, bool isOk) {
    return AnimatedCard(
      padding: const EdgeInsets.all(8),
      child: SizedBox(
        height: 60,
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Text(
              label,
              style: const TextStyle(
                color: Colors.white,
                fontSize: 12,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 4),
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                PulsingDot(
                  color: isOk ? Colors.green : Colors.red,
                  size: 6,
                  duration: isOk 
                      ? const Duration(seconds: 2) 
                      : const Duration(milliseconds: 500),
                ),
                const SizedBox(width: 6),
                Text(
                  isOk ? 'OK' : 'ERR',
                  style: TextStyle(
                    color: isOk ? Colors.green : Colors.red,
                    fontSize: 12,
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

  Widget _buildActionButton(String text, Color color, IconData icon) {
    return Container(
      height: 50,
      decoration: BoxDecoration(
        gradient: LinearGradient(
          colors: [color, color.withOpacity(0.8)],
        ),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          Icon(icon, color: Colors.white, size: 18),
          const SizedBox(width: 8),
          Text(
            text,
            style: const TextStyle(
              color: Colors.white,
              fontSize: 12,
              fontWeight: FontWeight.bold,
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildCodeExample(String code) {
    return Container(
      width: double.infinity,
      padding: const EdgeInsets.all(12),
      decoration: BoxDecoration(
        color: Colors.black.withOpacity(0.6),
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: Colors.grey.withOpacity(0.3)),
      ),
      child: Text(
        code,
        style: const TextStyle(
          color: Colors.white,
          fontFamily: 'monospace',
          fontSize: 10,
        ),
      ),
    );
  }
}