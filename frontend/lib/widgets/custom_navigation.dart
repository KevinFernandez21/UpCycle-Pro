import 'package:flutter/material.dart';
import 'animated_widgets.dart';

class CustomNavigation extends StatefulWidget {
  final int selectedIndex;
  final Function(int) onTabTapped;

  const CustomNavigation({
    super.key,
    required this.selectedIndex,
    required this.onTabTapped,
  });

  @override
  State<CustomNavigation> createState() => _CustomNavigationState();
}

class _CustomNavigationState extends State<CustomNavigation>
    with TickerProviderStateMixin {
  late AnimationController _controller;
  late Animation<double> _scaleAnimation;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      duration: const Duration(milliseconds: 300),
      vsync: this,
    );
    _scaleAnimation = Tween<double>(
      begin: 1.0,
      end: 1.1,
    ).animate(CurvedAnimation(
      parent: _controller,
      curve: Curves.elasticOut,
    ));
  }

  @override
  void didUpdateWidget(CustomNavigation oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.selectedIndex != widget.selectedIndex) {
      _controller.forward().then((_) => _controller.reverse());
    }
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      height: 80,
      decoration: BoxDecoration(
        color: Colors.black.withOpacity(0.4),
        border: Border(
          bottom: BorderSide(color: Colors.white.withOpacity(0.1)),
        ),
      ),
      child: Row(
        children: [
          _buildNavTab('[H]', 'HOME', 0),
          _buildNavTab('[C]', 'CÁMARA', 1),
          _buildNavTab('[M]', 'MONITOR', 2),
          _buildNavTab('[*]', 'CONFIG', 3),
        ],
      ),
    );
  }

  Widget _buildNavTab(String emoji, String text, int index) {
    final isActive = widget.selectedIndex == index;
    return Expanded(
      child: GestureDetector(
        onTap: () {
          widget.onTabTapped(index);
          _controller.forward().then((_) => _controller.reverse());
        },
        child: AnimatedContainer(
          duration: const Duration(milliseconds: 300),
          curve: Curves.easeInOut,
          decoration: BoxDecoration(
            color: isActive ? const Color(0xFF0064c8).withOpacity(0.3) : null,
            border: isActive
                ? const Border(
                    bottom: BorderSide(color: Color(0xFF00aaff), width: 3),
                  )
                : null,
          ),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              AnimatedBuilder(
                animation: _scaleAnimation,
                builder: (context, child) {
                  return Transform.scale(
                    scale: isActive ? _scaleAnimation.value : 1.0,
                    child: AnimatedContainer(
                      duration: const Duration(milliseconds: 300),
                      width: 40,
                      height: 40,
                      decoration: BoxDecoration(
                        border: Border.all(
                          color: isActive ? const Color(0xFF00aaff) : Colors.white.withOpacity(0.3),
                          width: 2,
                        ),
                        borderRadius: BorderRadius.circular(4),
                        color: isActive ? const Color(0xFF00aaff).withOpacity(0.2) : null,
                        boxShadow: isActive
                            ? [
                                BoxShadow(
                                  color: const Color(0xFF00aaff).withOpacity(0.4),
                                  blurRadius: 8,
                                  spreadRadius: 2,
                                ),
                              ]
                            : null,
                      ),
                      child: Center(
                        child: AnimatedDefaultTextStyle(
                          duration: const Duration(milliseconds: 300),
                          style: TextStyle(
                            color: isActive ? Colors.white : Colors.grey,
                            fontSize: 16,
                            fontWeight: FontWeight.bold,
                            fontFamily: 'monospace',
                          ),
                          child: Text(emoji),
                        ),
                      ),
                    ),
                  );
                },
              ),
              const SizedBox(height: 4),
              AnimatedDefaultTextStyle(
                duration: const Duration(milliseconds: 300),
                style: TextStyle(
                  color: isActive ? Colors.white : Colors.grey,
                  fontSize: 12,
                  fontWeight: FontWeight.bold,
                ),
                child: Text(text),
              ),
            ],
          ),
        ),
      ),
    );
  }
}