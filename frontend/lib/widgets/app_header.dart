import 'package:flutter/material.dart';
import 'animated_widgets.dart';

class AppHeader extends StatelessWidget {
  final String currentTime;

  const AppHeader({
    super.key,
    required this.currentTime,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: Colors.black.withOpacity(0.3),
        border: Border(
          bottom: BorderSide(color: Colors.white.withOpacity(0.1)),
        ),
      ),
      child: SafeArea(
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Row(
              children: [
                const SlideInAnimation(
                  begin: Offset(-0.5, 0),
                  duration: Duration(milliseconds: 1000),
                  child: Text(
                    'TFT-Universal',
                    style: TextStyle(
                      color: Colors.white,
                      fontSize: 20,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                ),
                const SizedBox(width: 16),
                const PulsingDot(
                  color: Colors.green,
                  size: 16,
                  duration: Duration(seconds: 2),
                ),
                const SizedBox(width: 8),
                const SlideInAnimation(
                  begin: Offset(-0.3, 0),
                  duration: Duration(milliseconds: 1200),
                  delay: Duration(milliseconds: 200),
                  child: Text(
                    'ONLINE',
                    style: TextStyle(
                      color: Colors.white,
                      fontSize: 18,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                ),
              ],
            ),
            SlideInAnimation(
              begin: const Offset(0.5, 0),
              duration: const Duration(milliseconds: 1000),
              delay: const Duration(milliseconds: 400),
              child: Text(
                currentTime,
                style: const TextStyle(
                  color: Colors.grey,
                  fontSize: 16,
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}