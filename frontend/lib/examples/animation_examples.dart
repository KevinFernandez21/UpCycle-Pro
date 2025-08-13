import 'package:flutter/material.dart';
import '../widgets/animated_widgets.dart';

/// Ejemplos de uso para todos los widgets animados
/// Ejecuta esta pantalla para ver todos los componentes en acción
class AnimationExamplesScreen extends StatefulWidget {
  const AnimationExamplesScreen({super.key});

  @override
  State<AnimationExamplesScreen> createState() => _AnimationExamplesScreenState();
}

class _AnimationExamplesScreenState extends State<AnimationExamplesScreen> {
  double _progressValue = 0.75;
  
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFF1a1a2e),
      appBar: AppBar(
        title: const Text('Ejemplos de Animaciones'),
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
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              _buildSection(
                '🔴 PulsingDot - Indicadores Pulsantes',
                _buildPulsingDotExamples(),
              ),
              _buildSection(
                '📊 AnimatedProgressBar - Barras de Progreso',
                _buildProgressBarExamples(),
              ),
              _buildSection(
                '🃏 AnimatedCard - Cards Interactivas',
                _buildAnimatedCardExamples(),
              ),
              _buildSection(
                '🎈 FloatingButton - Botones Flotantes',
                _buildFloatingButtonExamples(),
              ),
              _buildSection(
                '🌟 RotatingIcon - Iconos Rotatorios',
                _buildRotatingIconExamples(),
              ),
              _buildSection(
                '✨ SlideInAnimation - Animaciones de Entrada',
                _buildSlideInExamples(),
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildSection(String title, Widget content) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          title,
          style: const TextStyle(
            color: Color(0xFF00aaff),
            fontSize: 20,
            fontWeight: FontWeight.bold,
          ),
        ),
        const SizedBox(height: 16),
        content,
        const SizedBox(height: 32),
      ],
    );
  }

  /// EJEMPLO 1: PulsingDot
  Widget _buildPulsingDotExamples() {
    return AnimatedCard(
      child: Column(
        children: [
          const Text(
            'Diferentes tipos de puntos pulsantes:',
            style: TextStyle(color: Colors.white, fontSize: 16),
          ),
          const SizedBox(height: 20),
          
          // Ejemplo básico
          Row(
            children: [
              const PulsingDot(
                color: Colors.green,
                size: 16,
                duration: Duration(seconds: 2),
              ),
              const SizedBox(width: 16),
              const Text(
                'Estado Online (Verde, lento)',
                style: TextStyle(color: Colors.white),
              ),
            ],
          ),
          const SizedBox(height: 16),
          
          // Ejemplo de error
          Row(
            children: [
              const PulsingDot(
                color: Colors.red,
                size: 16,
                duration: Duration(milliseconds: 500),
              ),
              const SizedBox(width: 16),
              const Text(
                'Estado Error (Rojo, rápido)',
                style: TextStyle(color: Colors.white),
              ),
            ],
          ),
          const SizedBox(height: 16),
          
          // Ejemplo personalizado
          Row(
            children: [
              const PulsingDot(
                color: Color(0xFF00aaff),
                size: 24,
                duration: Duration(milliseconds: 1000),
              ),
              const SizedBox(width: 16),
              const Text(
                'Indicador personalizado (Azul, grande)',
                style: TextStyle(color: Colors.white),
              ),
            ],
          ),
          
          const SizedBox(height: 20),
          const Text(
            'Código de ejemplo:',
            style: TextStyle(color: Colors.yellow, fontWeight: FontWeight.bold),
          ),
          const SizedBox(height: 8),
          Container(
            padding: const EdgeInsets.all(12),
            decoration: BoxDecoration(
              color: Colors.black.withOpacity(0.6),
              borderRadius: BorderRadius.circular(8),
            ),
            child: const Text(
              '''PulsingDot(
  color: Colors.green,
  size: 16,
  duration: Duration(seconds: 2),
)''',
              style: TextStyle(
                color: Colors.white,
                fontFamily: 'monospace',
                fontSize: 12,
              ),
            ),
          ),
        ],
      ),
    );
  }

  /// EJEMPLO 2: AnimatedProgressBar
  Widget _buildProgressBarExamples() {
    return AnimatedCard(
      child: Column(
        children: [
          const Text(
            'Barras de progreso animadas:',
            style: TextStyle(color: Colors.white, fontSize: 16),
          ),
          const SizedBox(height: 20),
          
          // CPU Progress
          AnimatedProgressBar(
            label: 'CPU Usage',
            value: '${(_progressValue * 100).toInt()}%',
            progress: _progressValue,
            color: Colors.red,
            duration: const Duration(milliseconds: 1500),
          ),
          const SizedBox(height: 16),
          
          // RAM Progress
          AnimatedProgressBar(
            label: 'Memory',
            value: '8.5 GB',
            progress: 0.85,
            color: const Color(0xFF4ecdc4),
            duration: const Duration(milliseconds: 2000),
          ),
          const SizedBox(height: 16),
          
          // Temperature Progress
          AnimatedProgressBar(
            label: 'Temperature',
            value: '42°C',
            progress: 0.60,
            color: const Color(0xFFff9a9e),
            duration: const Duration(milliseconds: 2500),
          ),
          
          const SizedBox(height: 20),
          ElevatedButton(
            onPressed: () {
              setState(() {
                _progressValue = _progressValue == 0.75 ? 0.45 : 0.75;
              });
            },
            child: const Text('Cambiar valor CPU'),
          ),
          
          const SizedBox(height: 20),
          const Text(
            'Código de ejemplo:',
            style: TextStyle(color: Colors.yellow, fontWeight: FontWeight.bold),
          ),
          const SizedBox(height: 8),
          Container(
            padding: const EdgeInsets.all(12),
            decoration: BoxDecoration(
              color: Colors.black.withOpacity(0.6),
              borderRadius: BorderRadius.circular(8),
            ),
            child: const Text(
              '''AnimatedProgressBar(
  label: 'CPU Usage',
  value: '75%',
  progress: 0.75,
  color: Colors.red,
  duration: Duration(milliseconds: 1500),
)''',
              style: TextStyle(
                color: Colors.white,
                fontFamily: 'monospace',
                fontSize: 12,
              ),
            ),
          ),
        ],
      ),
    );
  }

  /// EJEMPLO 3: AnimatedCard
  Widget _buildAnimatedCardExamples() {
    return Column(
      children: [
        Row(
          children: [
            Expanded(
              child: AnimatedCard(
                onTap: () {
                  ScaffoldMessenger.of(context).showSnackBar(
                    const SnackBar(content: Text('Card básica presionada!')),
                  );
                },
                child: const Column(
                  children: [
                    Icon(Icons.computer, color: Colors.blue, size: 32),
                    SizedBox(height: 8),
                    Text(
                      'Sistema',
                      style: TextStyle(color: Colors.white, fontSize: 16),
                    ),
                    Text(
                      'Presiona aquí',
                      style: TextStyle(color: Colors.grey, fontSize: 12),
                    ),
                  ],
                ),
              ),
            ),
            const SizedBox(width: 16),
            Expanded(
              child: AnimatedCard(
                onTap: () {
                  ScaffoldMessenger.of(context).showSnackBar(
                    const SnackBar(content: Text('Card de estado presionada!')),
                  );
                },
                child: const Column(
                  children: [
                    Icon(Icons.check_circle, color: Colors.green, size: 32),
                    SizedBox(height: 8),
                    Text(
                      'Estado',
                      style: TextStyle(color: Colors.white, fontSize: 16),
                    ),
                    Text(
                      'Online',
                      style: TextStyle(color: Colors.green, fontSize: 12),
                    ),
                  ],
                ),
              ),
            ),
          ],
        ),
        const SizedBox(height: 20),
        const Text(
          'Código de ejemplo:',
          style: TextStyle(color: Colors.yellow, fontWeight: FontWeight.bold),
        ),
        const SizedBox(height: 8),
        Container(
          padding: const EdgeInsets.all(12),
          decoration: BoxDecoration(
            color: Colors.black.withOpacity(0.6),
            borderRadius: BorderRadius.circular(8),
          ),
          child: const Text(
            '''AnimatedCard(
  onTap: () {
    print('Card presionada!');
  },
  child: Column(
    children: [
      Icon(Icons.computer, color: Colors.blue),
      Text('Sistema'),
    ],
  ),
)''',
            style: TextStyle(
              color: Colors.white,
              fontFamily: 'monospace',
              fontSize: 12,
            ),
          ),
        ),
      ],
    );
  }

  /// EJEMPLO 4: FloatingButton
  Widget _buildFloatingButtonExamples() {
    return AnimatedCard(
      child: Column(
        children: [
          const Text(
            'Botones con animación flotante:',
            style: TextStyle(color: Colors.white, fontSize: 16),
          ),
          const SizedBox(height: 20),
          
          Row(
            children: [
              Expanded(
                child: FloatingButton(
                  duration: const Duration(seconds: 3),
                  onPressed: () {
                    ScaffoldMessenger.of(context).showSnackBar(
                      const SnackBar(content: Text('Botón Lento presionado!')),
                    );
                  },
                  child: Container(
                    height: 50,
                    decoration: BoxDecoration(
                      gradient: const LinearGradient(
                        colors: [Color(0xFF667eea), Color(0xFF764ba2)],
                      ),
                      borderRadius: BorderRadius.circular(8),
                    ),
                    child: const Center(
                      child: Text(
                        'Lento (3s)',
                        style: TextStyle(color: Colors.white),
                      ),
                    ),
                  ),
                ),
              ),
              const SizedBox(width: 16),
              Expanded(
                child: FloatingButton(
                  duration: const Duration(seconds: 1),
                  onPressed: () {
                    ScaffoldMessenger.of(context).showSnackBar(
                      const SnackBar(content: Text('Botón Rápido presionado!')),
                    );
                  },
                  child: Container(
                    height: 50,
                    decoration: BoxDecoration(
                      gradient: const LinearGradient(
                        colors: [Color(0xFF4ecdc4), Color(0xFF44a08d)],
                      ),
                      borderRadius: BorderRadius.circular(8),
                    ),
                    child: const Center(
                      child: Text(
                        'Rápido (1s)',
                        style: TextStyle(color: Colors.white),
                      ),
                    ),
                  ),
                ),
              ),
            ],
          ),
          
          const SizedBox(height: 20),
          const Text(
            'Código de ejemplo:',
            style: TextStyle(color: Colors.yellow, fontWeight: FontWeight.bold),
          ),
          const SizedBox(height: 8),
          Container(
            padding: const EdgeInsets.all(12),
            decoration: BoxDecoration(
              color: Colors.black.withOpacity(0.6),
              borderRadius: BorderRadius.circular(8),
            ),
            child: const Text(
              '''FloatingButton(
  duration: Duration(seconds: 3),
  onPressed: () => print('Pressed!'),
  child: Container(
    decoration: BoxDecoration(
      gradient: LinearGradient(
        colors: [Color(0xFF667eea), Color(0xFF764ba2)],
      ),
    ),
    child: Text('Mi Botón'),
  ),
)''',
              style: TextStyle(
                color: Colors.white,
                fontFamily: 'monospace',
                fontSize: 12,
              ),
            ),
          ),
        ],
      ),
    );
  }

  /// EJEMPLO 5: RotatingIcon
  Widget _buildRotatingIconExamples() {
    return AnimatedCard(
      child: Column(
        children: [
          const Text(
            'Iconos con rotación continua:',
            style: TextStyle(color: Colors.white, fontSize: 16),
          ),
          const SizedBox(height: 20),
          
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceEvenly,
            children: [
              Column(
                children: [
                  const RotatingIcon(
                    icon: Icons.settings,
                    color: Colors.grey,
                    size: 32,
                    duration: Duration(seconds: 4),
                  ),
                  const SizedBox(height: 8),
                  const Text(
                    'Settings',
                    style: TextStyle(color: Colors.white, fontSize: 12),
                  ),
                ],
              ),
              Column(
                children: [
                  const RotatingIcon(
                    icon: Icons.refresh,
                    color: Color(0xFF00aaff),
                    size: 32,
                    duration: Duration(seconds: 2),
                  ),
                  const SizedBox(height: 8),
                  const Text(
                    'Refresh',
                    style: TextStyle(color: Colors.white, fontSize: 12),
                  ),
                ],
              ),
              Column(
                children: [
                  const RotatingIcon(
                    icon: Icons.videocam,
                    color: Colors.green,
                    size: 32,
                    duration: Duration(seconds: 6),
                  ),
                  const SizedBox(height: 8),
                  const Text(
                    'Camera',
                    style: TextStyle(color: Colors.white, fontSize: 12),
                  ),
                ],
              ),
            ],
          ),
          
          const SizedBox(height: 20),
          const Text(
            'Código de ejemplo:',
            style: TextStyle(color: Colors.yellow, fontWeight: FontWeight.bold),
          ),
          const SizedBox(height: 8),
          Container(
            padding: const EdgeInsets.all(12),
            decoration: BoxDecoration(
              color: Colors.black.withOpacity(0.6),
              borderRadius: BorderRadius.circular(8),
            ),
            child: const Text(
              '''RotatingIcon(
  icon: Icons.settings,
  color: Colors.grey,
  size: 32,
  duration: Duration(seconds: 4),
)''',
              style: TextStyle(
                color: Colors.white,
                fontFamily: 'monospace',
                fontSize: 12,
              ),
            ),
          ),
        ],
      ),
    );
  }

  /// EJEMPLO 6: SlideInAnimation
  Widget _buildSlideInExamples() {
    return AnimatedCard(
      child: Column(
        children: [
          const Text(
            'Animaciones de entrada deslizante:',
            style: TextStyle(color: Colors.white, fontSize: 16),
          ),
          const SizedBox(height: 20),
          
          // Desde la izquierda
          SlideInAnimation(
            begin: const Offset(-0.5, 0),
            duration: const Duration(milliseconds: 800),
            delay: const Duration(milliseconds: 0),
            child: Container(
              padding: const EdgeInsets.all(12),
              margin: const EdgeInsets.only(bottom: 12),
              decoration: BoxDecoration(
                color: Colors.blue.withOpacity(0.3),
                borderRadius: BorderRadius.circular(8),
              ),
              child: const Text(
                '← Desde la izquierda',
                style: TextStyle(color: Colors.white),
              ),
            ),
          ),
          
          // Desde la derecha
          SlideInAnimation(
            begin: const Offset(0.5, 0),
            duration: const Duration(milliseconds: 800),
            delay: const Duration(milliseconds: 200),
            child: Container(
              padding: const EdgeInsets.all(12),
              margin: const EdgeInsets.only(bottom: 12),
              decoration: BoxDecoration(
                color: Colors.green.withOpacity(0.3),
                borderRadius: BorderRadius.circular(8),
              ),
              child: const Text(
                'Desde la derecha →',
                style: TextStyle(color: Colors.white),
              ),
            ),
          ),
          
          // Desde abajo
          SlideInAnimation(
            begin: const Offset(0, 0.5),
            duration: const Duration(milliseconds: 800),
            delay: const Duration(milliseconds: 400),
            child: Container(
              padding: const EdgeInsets.all(12),
              margin: const EdgeInsets.only(bottom: 12),
              decoration: BoxDecoration(
                color: Colors.purple.withOpacity(0.3),
                borderRadius: BorderRadius.circular(8),
              ),
              child: const Text(
                '↑ Desde abajo',
                style: TextStyle(color: Colors.white),
              ),
            ),
          ),
          
          const SizedBox(height: 20),
          const Text(
            'Código de ejemplo:',
            style: TextStyle(color: Colors.yellow, fontWeight: FontWeight.bold),
          ),
          const SizedBox(height: 8),
          Container(
            padding: const EdgeInsets.all(12),
            decoration: BoxDecoration(
              color: Colors.black.withOpacity(0.6),
              borderRadius: BorderRadius.circular(8),
            ),
            child: const Text(
              '''SlideInAnimation(
  begin: Offset(-0.5, 0), // Desde izquierda
  duration: Duration(milliseconds: 800),
  delay: Duration(milliseconds: 200),
  child: Text('Mi contenido'),
)''',
              style: TextStyle(
                color: Colors.white,
                fontFamily: 'monospace',
                fontSize: 12,
              ),
            ),
          ),
        ],
      ),
    );
  }
}