import 'package:flutter/material.dart';
import '../widgets/animated_widgets.dart';
import '../services/api_service.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  final ApiService _apiService = ApiService();
  Map<String, dynamic>? _apiInfo;
  Map<String, dynamic>? _modelInfo;
  bool _isLoading = true;

  @override
  void initState() {
    super.initState();
    _loadApiData();
  }

  Future<void> _loadApiData() async {
    try {
      final apiInfoFuture = _apiService.getApiInfo();
      final modelInfoFuture = _apiService.getModelInfo();

      final results = await Future.wait([
        apiInfoFuture,
        modelInfoFuture.catchError((e) => null),
      ]);

      setState(() {
        _apiInfo = results[0] as Map<String, dynamic>?;
        _modelInfo = results[1] as Map<String, dynamic>?;
        _isLoading = false;
      });
    } catch (e) {
      setState(() {
        _isLoading = false;
      });
    }
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
          AnimatedProgressBar(
            label: 'CPU',
            value: '65%',
            progress: 0.65,
            color: Colors.red,
            duration: const Duration(milliseconds: 2000),
          ),
          const SizedBox(height: 12),
          AnimatedProgressBar(
            label: 'RAM',
            value: '90%',
            progress: 0.90,
            color: const Color(0xFF4ecdc4),
            duration: const Duration(milliseconds: 2500),
          ),
          const SizedBox(height: 12),
          AnimatedProgressBar(
            label: 'Temp',
            value: '42°C',
            progress: 0.70,
            color: const Color(0xFFff9a9e),
            duration: const Duration(milliseconds: 3000),
          ),
        ],
      ),
    );
  }

  Widget _buildDetectionCard() {
    return AnimatedCard(
      child: Column(
        children: [
          const Text(
            '[%] Detección',
            style: TextStyle(
              color: Color(0xFF00aaff),
              fontSize: 18,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 20),
          const SlideInAnimation(
            begin: Offset(0, 0.3),
            duration: Duration(milliseconds: 1500),
            delay: Duration(milliseconds: 800),
            child: Text(
              'Plástico',
              style: TextStyle(
                color: Color(0xFFfeca57),
                fontSize: 32,
                fontWeight: FontWeight.bold,
              ),
            ),
          ),
          SlideInAnimation(
            begin: const Offset(0, 0.3),
            duration: const Duration(milliseconds: 1200),
            delay: const Duration(milliseconds: 1200),
            child: const Text(
              '95.8%',
              style: TextStyle(
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
                      border: Border.all(color: Colors.green, width: 2),
                      boxShadow: [
                        BoxShadow(
                          color: Colors.green.withOpacity(0.3),
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
                          _modelInfo != null ? 'Cargado' : 'No Cargado',
                          style: TextStyle(
                            color: _modelInfo != null ? const Color(0xFF00aaff) : Colors.red,
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
                      border: Border.all(color: Colors.green, width: 2),
                      boxShadow: [
                        BoxShadow(
                          color: Colors.green.withOpacity(0.3),
                          blurRadius: 8,
                        ),
                      ],
                    ),
                    child: const Column(
                      children: [
                        Text(
                          'RNN',
                          style: TextStyle(
                            color: Colors.white,
                            fontSize: 18,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                        SizedBox(height: 8),
                        Text(
                          '92.5%',
                          style: TextStyle(
                            color: Color(0xFF00aaff),
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
                        _apiInfo != null ? 'FastAPI Conectado' : 'FastAPI Desconectado',
                        style: const TextStyle(color: Colors.white, fontSize: 14),
                      ),
                      PulsingDot(
                        color: _apiInfo != null ? Colors.green : Colors.red,
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
                        _apiInfo != null ? 'Estado: ${_apiInfo!['status'] ?? 'N/A'}' : 'Estado: Offline',
                        style: const TextStyle(color: Colors.white, fontSize: 14),
                      ),
                      Text(
                        _apiInfo != null ? 'v${_apiInfo!['version'] ?? '1.0.0'}' : 'N/A',
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
              Expanded(
                child: SlideInAnimation(
                  begin: const Offset(-0.3, 0),
                  duration: const Duration(milliseconds: 600),
                  delay: const Duration(milliseconds: 200),
                  child: _buildSensor('PIR-1', 'OK', true),
                ),
              ),
              Expanded(
                child: SlideInAnimation(
                  begin: const Offset(-0.1, 0),
                  duration: const Duration(milliseconds: 600),
                  delay: const Duration(milliseconds: 400),
                  child: _buildSensor('PIR-2', 'OK', true),
                ),
              ),
              Expanded(
                child: SlideInAnimation(
                  begin: const Offset(0.1, 0),
                  duration: const Duration(milliseconds: 600),
                  delay: const Duration(milliseconds: 600),
                  child: _buildSensor('PIR-3', 'ERR', false),
                ),
              ),
              Expanded(
                child: SlideInAnimation(
                  begin: const Offset(0.3, 0),
                  duration: const Duration(milliseconds: 600),
                  delay: const Duration(milliseconds: 800),
                  child: _buildSensor('PIR-4', 'OK', true),
                ),
              ),
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
                if (isOk)
                  const PulsingDot(
                    color: Colors.green,
                    size: 8,
                    duration: Duration(seconds: 2),
                  )
                else
                  const PulsingDot(
                    color: Colors.red,
                    size: 8,
                    duration: Duration(milliseconds: 500),
                  ),
                const SizedBox(width: 8),
                Text(
                  value,
                  style: TextStyle(
                    color: isOk ? Colors.green : Colors.red,
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