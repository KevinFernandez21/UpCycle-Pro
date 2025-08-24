import 'package:flutter/material.dart';
import 'dart:math' as math;

/// Widget optimizado para Raspberry Pi 5 - Sin setState durante layout
class RaspberryPiLoader extends StatelessWidget {
  final String message;
  final double size;
  final Color color;
  final bool showMessage;

  const RaspberryPiLoader({
    super.key,
    this.message = 'Cargando...',
    this.size = 48.0,
    this.color = const Color(0xFF00aaff),
    this.showMessage = true,
  });

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          // Simple circular progress indicator
          SizedBox(
            width: size,
            height: size,
            child: CircularProgressIndicator(
              strokeWidth: 3,
              valueColor: AlwaysStoppedAnimation<Color>(color),
              backgroundColor: color.withOpacity(0.1),
            ),
          ),
          
          if (showMessage) ...[
            const SizedBox(height: 16),
            Text(
              message,
              style: const TextStyle(
                color: Colors.white70,
                fontSize: 14,
                fontWeight: FontWeight.w500,
              ),
              textAlign: TextAlign.center,
            ),
          ],
        ],
      ),
    );
  }
}

/// Widget de progreso optimizado sin animaciones complejas
class OptimizedProgressBar extends StatelessWidget {
  final double progress;
  final String label;
  final Color color;

  const OptimizedProgressBar({
    super.key,
    required this.progress,
    required this.label,
    this.color = const Color(0xFF00aaff),
  });

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          label,
          style: const TextStyle(
            color: Colors.white,
            fontSize: 12,
            fontWeight: FontWeight.w500,
          ),
        ),
        const SizedBox(height: 8),
        Container(
          height: 8,
          decoration: BoxDecoration(
            color: Colors.white.withOpacity(0.1),
            borderRadius: BorderRadius.circular(4),
          ),
          child: FractionallySizedBox(
            alignment: Alignment.centerLeft,
            widthFactor: progress.clamp(0.0, 1.0),
            child: Container(
              decoration: BoxDecoration(
                color: color,
                borderRadius: BorderRadius.circular(4),
              ),
            ),
          ),
        ),
      ],
    );
  }
}

/// Skeleton loader simple sin animaciones
class SkeletonLoader extends StatelessWidget {
  final double width;
  final double height;
  final BorderRadius? borderRadius;

  const SkeletonLoader({
    super.key,
    required this.width,
    required this.height,
    this.borderRadius,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      width: width,
      height: height,
      decoration: BoxDecoration(
        color: Colors.white.withOpacity(0.1),
        borderRadius: borderRadius ?? BorderRadius.circular(4),
      ),
    );
  }
}

/// Estado de carga para pantallas completas
class LoadingOverlay extends StatelessWidget {
  final String message;
  final bool isVisible;
  final Widget child;

  const LoadingOverlay({
    super.key,
    required this.child,
    this.message = 'Cargando datos del sistema...',
    this.isVisible = false,
  });

  @override
  Widget build(BuildContext context) {
    return Stack(
      children: [
        child,
        if (isVisible)
          Container(
            color: Colors.black.withOpacity(0.7),
            child: RaspberryPiLoader(
              message: message,
              size: 64,
            ),
          ),
      ],
    );
  }
}

/// Widget de conexión simple sin animaciones
class ConnectionStatus extends StatelessWidget {
  final bool isConnected;
  final String label;
  final VoidCallback? onRetry;

  const ConnectionStatus({
    super.key,
    required this.isConnected,
    required this.label,
    this.onRetry,
  });

  @override
  Widget build(BuildContext context) {
    final color = isConnected 
        ? const Color(0xFF00C851) 
        : const Color(0xFFFF3547);

    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
      decoration: BoxDecoration(
        color: color.withOpacity(0.1),
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: color.withOpacity(0.3)),
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          Container(
            width: 8,
            height: 8,
            decoration: BoxDecoration(
              shape: BoxShape.circle,
              color: color,
            ),
          ),
          const SizedBox(width: 8),
          Expanded(
            child: Text(
              label,
              style: TextStyle(
                color: color,
                fontSize: 12,
                fontWeight: FontWeight.w600,
              ),
            ),
          ),
          if (!isConnected && onRetry != null) ...[
            const SizedBox(width: 8),
            GestureDetector(
              onTap: onRetry,
              child: Icon(
                Icons.refresh,
                size: 16,
                color: color,
              ),
            ),
          ],
        ],
      ),
    );
  }
}