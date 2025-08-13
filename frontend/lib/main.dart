import 'package:flutter/material.dart';
import 'dart:async';
import 'screens/home_screen.dart';
import 'screens/camera_screen.dart';
import 'screens/config_screen.dart';
import 'screens/monitoring_screen.dart';
import 'widgets/app_header.dart';
import 'widgets/custom_navigation.dart';

void main() {
  runApp(const UpCycleProApp());
}

class UpCycleProApp extends StatelessWidget {
  const UpCycleProApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'UpCycle Pro',
      theme: ThemeData(
        primarySwatch: Colors.blue,
        fontFamily: 'Arial',
        scaffoldBackgroundColor: const Color(0xFF1a1a2e),
        textTheme: const TextTheme(
          bodyLarge: TextStyle(color: Colors.white, fontWeight: FontWeight.bold),
          bodyMedium: TextStyle(color: Colors.white, fontWeight: FontWeight.bold),
          titleLarge: TextStyle(color: Color(0xFF00aaff), fontWeight: FontWeight.bold),
        ),
      ),
      home: const MainScreen(),
    );
  }
}

class MainScreen extends StatefulWidget {
  const MainScreen({super.key});

  @override
  State<MainScreen> createState() => _MainScreenState();
}

class _MainScreenState extends State<MainScreen> with TickerProviderStateMixin {
  int _selectedIndex = 0;
  late Timer _timer;
  String _currentTime = '';
  
  final PageController _pageController = PageController();

  @override
  void initState() {
    super.initState();
    _updateTime();
    _timer = Timer.periodic(const Duration(seconds: 1), (timer) {
      _updateTime();
    });
  }

  @override
  void dispose() {
    _timer.cancel();
    _pageController.dispose();
    super.dispose();
  }

  void _updateTime() {
    final now = DateTime.now();
    setState(() {
      _currentTime = '${now.hour}:${now.minute.toString().padLeft(2, '0')}:${now.second.toString().padLeft(2, '0')} ${now.hour >= 12 ? 'PM' : 'AM'}';
    });
  }

  void _onTabTapped(int index) {
    setState(() {
      _selectedIndex = index;
    });
    _pageController.animateToPage(
      index,
      duration: const Duration(milliseconds: 300),
      curve: Curves.easeInOut,
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Container(
        decoration: const BoxDecoration(
          gradient: LinearGradient(
            colors: [Color(0xFF1a1a2e), Color(0xFF16213e), Color(0xFF0f3460)],
            begin: Alignment.topLeft,
            end: Alignment.bottomRight,
          ),
        ),
        child: Column(
          children: [
            AppHeader(currentTime: _currentTime),
            CustomNavigation(
              selectedIndex: _selectedIndex,
              onTabTapped: _onTabTapped,
            ),
            Expanded(
              child: PageView(
                controller: _pageController,
                onPageChanged: (index) {
                  setState(() {
                    _selectedIndex = index;
                  });
                },
                children: const [
                  HomeScreen(),
                  CameraScreen(),
                  MonitoringScreen(),
                  ConfigScreen(),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}