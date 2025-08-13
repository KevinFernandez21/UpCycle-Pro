import 'package:flutter/material.dart';
import 'dart:math' as math;

// Paleta de colores optimizada para pantalla 480x320
class AppColors {
  static const primaryGreen = Color(0xFF2ECC71);
  static const primaryBlue = Color(0xFF3498DB);
  static const accentOrange = Color(0xFFE67E22);
  static const accentPurple = Color(0xFF9B59B6);
  static const darkBase = Color(0xFF1A1A1A);
  static const mediumDark = Color(0xFF2C2C2C);
  static const lightGray = Color(0xFF3A3A3A);
  static const softWhite = Color(0xFFF8F9FA);
  static const errorRed = Color(0xFFE74C3C);
  static const warningYellow = Color(0xFFF39C12);
}

class PulsingDot extends StatefulWidget {
  final Color color;
  final double size;
  final Duration duration;

  const PulsingDot({
    super.key,
    this.color = AppColors.primaryGreen,
    this.size = 12,
    this.duration = const Duration(seconds: 2),
  });

  @override
  State<PulsingDot> createState() => _PulsingDotState();
}

class _PulsingDotState extends State<PulsingDot>
    with SingleTickerProviderStateMixin {
  late AnimationController _animationController;
  late Animation<double> _animation;

  @override
  void initState() {
    super.initState();
    _animationController = AnimationController(
      duration: widget.duration,
      vsync: this,
    );
    _animation = Tween<double>(
      begin: 0.0,
      end: 1.0,
    ).animate(CurvedAnimation(
      parent: _animationController,
      curve: Curves.easeInOut,
    ));
    _animationController.repeat(reverse: true);
  }

  @override
  void dispose() {
    _animationController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: _animation,
      builder: (context, child) {
        return Container(
          width: widget.size,
          height: widget.size,
          decoration: BoxDecoration(
            gradient: RadialGradient(
              colors: [
                widget.color,
                widget.color.withOpacity(0.8),
                widget.color.withOpacity(0.4),
              ],
              stops: [0.0, 0.6, 1.0],
            ),
            borderRadius: BorderRadius.circular(widget.size / 2),
            boxShadow: [
              BoxShadow(
                color: widget.color.withOpacity(0.4 * _animation.value),
                blurRadius: 8 + (6 * _animation.value),
                spreadRadius: 1 + (3 * _animation.value),
              ),
            ],
          ),
          child: Container(
            margin: EdgeInsets.all(1.5),
            decoration: BoxDecoration(
              color: widget.color,
              borderRadius: BorderRadius.circular((widget.size - 3) / 2),
              boxShadow: [
                BoxShadow(
                  color: AppColors.softWhite.withOpacity(0.2 * _animation.value),
                  blurRadius: 2,
                  offset: Offset(-0.5, -0.5),
                ),
              ],
            ),
          ),
        );
      },
    );
  }
}

class AnimatedProgressBar extends StatefulWidget {
  final String label;
  final String value;
  final double progress;
  final Color color;
  final Duration duration;

  const AnimatedProgressBar({
    super.key,
    required this.label,
    required this.value,
    required this.progress,
    this.color = AppColors.primaryBlue,
    this.duration = const Duration(milliseconds: 1200),
  });

  @override
  State<AnimatedProgressBar> createState() => _AnimatedProgressBarState();
}

class _AnimatedProgressBarState extends State<AnimatedProgressBar>
    with SingleTickerProviderStateMixin {
  late AnimationController _controller;
  late Animation<double> _animation;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      duration: widget.duration,
      vsync: this,
    );
    _animation = Tween<double>(
      begin: 0.0,
      end: widget.progress,
    ).animate(CurvedAnimation(
      parent: _controller,
      curve: Curves.easeOutCubic,
    ));
    _controller.forward();
  }

  @override
  void didUpdateWidget(AnimatedProgressBar oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.progress != widget.progress) {
      _animation = Tween<double>(
        begin: _animation.value,
        end: widget.progress,
      ).animate(CurvedAnimation(
        parent: _controller,
        curve: Curves.easeOutCubic,
      ));
      _controller.reset();
      _controller.forward();
    }
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Flexible(
              child: Text(
                widget.label,
                style: const TextStyle(
                  color: AppColors.softWhite,
                  fontSize: 12,
                  fontWeight: FontWeight.w600,
                ),
                overflow: TextOverflow.ellipsis,
              ),
            ),
            Container(
              padding: const EdgeInsets.symmetric(horizontal: 6, vertical: 2),
              decoration: BoxDecoration(
                gradient: LinearGradient(
                  colors: [
                    widget.color.withOpacity(0.2),
                    widget.color.withOpacity(0.1),
                  ],
                ),
                borderRadius: BorderRadius.circular(8),
                border: Border.all(
                  color: widget.color.withOpacity(0.3),
                  width: 0.5,
                ),
              ),
              child: Text(
                widget.value,
                style: TextStyle(
                  color: widget.color,
                  fontSize: 10,
                  fontWeight: FontWeight.bold,
                ),
              ),
            ),
          ],
        ),
        const SizedBox(height: 6),
        Container(
          height: 16,
          decoration: BoxDecoration(
            gradient: LinearGradient(
              begin: Alignment.topCenter,
              end: Alignment.bottomCenter,
              colors: [
                AppColors.lightGray,
                AppColors.mediumDark,
              ],
            ),
            borderRadius: BorderRadius.circular(10),
            border: Border.all(
              color: AppColors.softWhite.withOpacity(0.1),
              width: 0.5,
            ),
          ),
          child: Padding(
            padding: const EdgeInsets.all(1),
            child: ClipRRect(
              borderRadius: BorderRadius.circular(8),
              child: AnimatedBuilder(
                animation: _animation,
                builder: (context, child) {
                  return LayoutBuilder(
                    builder: (context, constraints) {
                      return Stack(
                        children: [
                          Container(
                            decoration: BoxDecoration(
                              gradient: LinearGradient(
                                begin: Alignment.topCenter,
                                end: Alignment.bottomCenter,
                                colors: [
                                  AppColors.darkBase.withOpacity(0.8),
                                  AppColors.darkBase,
                                ],
                              ),
                              borderRadius: BorderRadius.circular(7),
                            ),
                          ),
                          Align(
                            alignment: Alignment.centerLeft,
                            child: Container(
                              width: constraints.maxWidth * _animation.value,
                              height: constraints.maxHeight,
                              decoration: BoxDecoration(
                                gradient: LinearGradient(
                                  begin: Alignment.centerLeft,
                                  end: Alignment.centerRight,
                                  colors: [
                                    widget.color,
                                    widget.color.withOpacity(0.9),
                                    widget.color,
                                  ],
                                  stops: const [0.0, 0.5, 1.0],
                                ),
                                borderRadius: BorderRadius.circular(7),
                              ),
                            ),
                          ),
                          if (_animation.value > 0.05)
                            Positioned(
                              left: 0,
                              top: 0,
                              bottom: 0,
                              child: Container(
                                width: constraints.maxWidth * _animation.value,
                                decoration: BoxDecoration(
                                  gradient: LinearGradient(
                                    begin: Alignment.topCenter,
                                    end: Alignment.bottomCenter,
                                    colors: [
                                      AppColors.softWhite.withOpacity(0.3),
                                      Colors.transparent,
                                      AppColors.darkBase.withOpacity(0.1),
                                    ],
                                    stops: const [0.0, 0.4, 1.0],
                                  ),
                                  borderRadius: BorderRadius.circular(7),
                                ),
                              ),
                            ),
                        ],
                      );
                    },
                  );
                },
              ),
            ),
          ),
        ),
      ],
    );
  }
}

class AnimatedCard extends StatefulWidget {
  final Widget child;
  final EdgeInsetsGeometry? padding;
  final double? width;
  final VoidCallback? onTap;
  final Color? backgroundColor;

  const AnimatedCard({
    super.key,
    required this.child,
    this.padding,
    this.width,
    this.onTap,
    this.backgroundColor,
  });

  @override
  State<AnimatedCard> createState() => _AnimatedCardState();
}

class _AnimatedCardState extends State<AnimatedCard>
    with SingleTickerProviderStateMixin {
  late AnimationController _controller;
  late Animation<double> _scaleAnimation;
  late Animation<double> _elevationAnimation;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      duration: const Duration(milliseconds: 150),
      vsync: this,
    );
    _scaleAnimation = Tween<double>(
      begin: 1.0,
      end: 0.98,
    ).animate(CurvedAnimation(
      parent: _controller,
      curve: Curves.easeInOut,
    ));
    _elevationAnimation = Tween<double>(
      begin: 0.0,
      end: 6.0,
    ).animate(CurvedAnimation(
      parent: _controller,
      curve: Curves.easeInOut,
    ));
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: _controller,
      builder: (context, child) {
        return GestureDetector(
          onTapDown: (_) {
            if (mounted) _controller.forward();
          },
          onTapUp: (_) {
            if (mounted) _controller.reverse();
          },
          onTapCancel: () {
            if (mounted) _controller.reverse();
          },
          onTap: widget.onTap,
          child: Transform.scale(
            scale: _scaleAnimation.value,
            child: Container(
              width: widget.width,
              padding: widget.padding ?? const EdgeInsets.all(12),
              decoration: BoxDecoration(
                gradient: LinearGradient(
                  begin: Alignment.topLeft,
                  end: Alignment.bottomRight,
                  colors: [
                    widget.backgroundColor?.withOpacity(0.15) ?? AppColors.mediumDark.withOpacity(0.9),
                    widget.backgroundColor?.withOpacity(0.1) ?? AppColors.darkBase.withOpacity(0.9),
                  ],
                ),
                border: Border.all(
                  color: AppColors.softWhite.withOpacity(0.1),
                  width: 0.5,
                ),
                borderRadius: BorderRadius.circular(12),
                boxShadow: [
                  BoxShadow(
                    color: AppColors.darkBase.withOpacity(0.3),
                    blurRadius: 4 + _elevationAnimation.value,
                    spreadRadius: 0.5 + (_elevationAnimation.value / 3),
                    offset: Offset(0, 2 + _elevationAnimation.value / 2),
                  ),
                  BoxShadow(
                    color: AppColors.softWhite.withOpacity(0.05),
                    blurRadius: 1,
                    offset: const Offset(0, -0.5),
                  ),
                ],
              ),
              child: widget.child,
            ),
          ),
        );
      },
    );
  }
}

class FloatingButton extends StatefulWidget {
  final Widget child;
  final VoidCallback? onPressed;
  final Duration duration;
  final Color? backgroundColor;

  const FloatingButton({
    super.key,
    required this.child,
    this.onPressed,
    this.duration = const Duration(seconds: 2),
    this.backgroundColor,
  });

  @override
  State<FloatingButton> createState() => _FloatingButtonState();
}

class _FloatingButtonState extends State<FloatingButton>
    with SingleTickerProviderStateMixin {
  late AnimationController _controller;
  late Animation<double> _animation;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      duration: widget.duration,
      vsync: this,
    );
    _animation = Tween<double>(
      begin: 0.0,
      end: 1.0,
    ).animate(CurvedAnimation(
      parent: _controller,
      curve: Curves.easeInOut,
    ));
    _controller.repeat(reverse: true);
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: widget.onPressed,
      child: AnimatedBuilder(
        animation: _animation,
        builder: (context, child) {
          return Transform.translate(
            offset: Offset(0, -3 * _animation.value),
            child: Container(
              decoration: BoxDecoration(
                gradient: RadialGradient(
                  colors: [
                    widget.backgroundColor ?? AppColors.primaryGreen,
                    (widget.backgroundColor ?? AppColors.primaryGreen).withOpacity(0.8),
                  ],
                ),
                borderRadius: BorderRadius.circular(25),
                boxShadow: [
                  BoxShadow(
                    color: (widget.backgroundColor ?? AppColors.primaryGreen).withOpacity(0.3),
                    blurRadius: 8 + (4 * _animation.value),
                    spreadRadius: 1 + (2 * _animation.value),
                    offset: Offset(0, 4 + (2 * _animation.value)),
                  ),
                ],
              ),
              child: widget.child,
            ),
          );
        },
      ),
    );
  }
}

class RotatingIcon extends StatefulWidget {
  final IconData icon;
  final Color color;
  final double size;
  final Duration duration;

  const RotatingIcon({
    super.key,
    required this.icon,
    this.color = AppColors.softWhite,
    this.size = 18,
    this.duration = const Duration(seconds: 3),
  });

  @override
  State<RotatingIcon> createState() => _RotatingIconState();
}

class _RotatingIconState extends State<RotatingIcon>
    with SingleTickerProviderStateMixin {
  late AnimationController _controller;
  late Animation<double> _animation;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      duration: widget.duration,
      vsync: this,
    );
    _animation = Tween<double>(
      begin: 0.0,
      end: 1.0,
    ).animate(_controller);
    _controller.repeat();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: _animation,
      builder: (context, child) {
        return Transform.rotate(
          angle: _animation.value * 2 * math.pi,
          child: Container(
            padding: EdgeInsets.all(6),
            decoration: BoxDecoration(
              gradient: RadialGradient(
                colors: [
                  widget.color.withOpacity(0.2),
                  widget.color.withOpacity(0.05),
                  Colors.transparent,
                ],
              ),
              shape: BoxShape.circle,
            ),
            child: Icon(
              widget.icon,
              color: widget.color,
              size: widget.size,
            ),
          ),
        );
      },
    );
  }
}

class SlideInAnimation extends StatefulWidget {
  final Widget child;
  final Duration duration;
  final Duration delay;
  final Offset begin;

  const SlideInAnimation({
    super.key,
    required this.child,
    this.duration = const Duration(milliseconds: 600),
    this.delay = Duration.zero,
    this.begin = const Offset(0, 0.3),
  });

  @override
  State<SlideInAnimation> createState() => _SlideInAnimationState();
}

class _SlideInAnimationState extends State<SlideInAnimation>
    with SingleTickerProviderStateMixin {
  late AnimationController _controller;
  late Animation<Offset> _slideAnimation;
  late Animation<double> _fadeAnimation;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      duration: widget.duration,
      vsync: this,
    );
    
    _slideAnimation = Tween<Offset>(
      begin: widget.begin,
      end: Offset.zero,
    ).animate(CurvedAnimation(
      parent: _controller,
      curve: Curves.easeOutCubic,
    ));
    
    _fadeAnimation = Tween<double>(
      begin: 0.0,
      end: 1.0,
    ).animate(CurvedAnimation(
      parent: _controller,
      curve: Curves.easeOut,
    ));

    Future.delayed(widget.delay, () {
      if (mounted) {
        _controller.forward();
      }
    });
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: _controller,
      builder: (context, child) {
        return SlideTransition(
          position: _slideAnimation,
          child: FadeTransition(
            opacity: _fadeAnimation,
            child: widget.child,
          ),
        );
      },
    );
  }
}

class GlowingBorder extends StatefulWidget {
  final Widget child;
  final Color glowColor;
  final double glowSize;
  final Duration duration;

  const GlowingBorder({
    super.key,
    required this.child,
    this.glowColor = AppColors.accentOrange,
    this.glowSize = 12,
    this.duration = const Duration(seconds: 1),
  });

  @override
  State<GlowingBorder> createState() => _GlowingBorderState();
}

class _GlowingBorderState extends State<GlowingBorder>
    with SingleTickerProviderStateMixin {
  late AnimationController _controller;
  late Animation<double> _animation;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      duration: widget.duration,
      vsync: this,
    );
    _animation = Tween<double>(
      begin: 0.4,
      end: 1.0,
    ).animate(CurvedAnimation(
      parent: _controller,
      curve: Curves.easeInOut,
    ));
    _controller.repeat(reverse: true);
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: _animation,
      builder: (context, child) {
        return Container(
          decoration: BoxDecoration(
            borderRadius: BorderRadius.circular(8),
            boxShadow: [
              BoxShadow(
                color: widget.glowColor.withOpacity(_animation.value * 0.4),
                blurRadius: widget.glowSize * _animation.value,
                spreadRadius: (widget.glowSize / 6) * _animation.value,
              ),
            ],
          ),
          child: widget.child,
        );
      },
    );
  }
}

class StatusIndicator extends StatefulWidget {
  final String status;
  final Color statusColor;
  final bool isActive;

  const StatusIndicator({
    super.key,
    required this.status,
    this.statusColor = AppColors.primaryGreen,
    this.isActive = true,
  });

  @override
  State<StatusIndicator> createState() => _StatusIndicatorState();
}

class _StatusIndicatorState extends State<StatusIndicator>
    with SingleTickerProviderStateMixin {
  late AnimationController _controller;
  late Animation<double> _animation;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      duration: const Duration(milliseconds: 1500),
      vsync: this,
    );
    _animation = Tween<double>(
      begin: 0.5,
      end: 1.0,
    ).animate(CurvedAnimation(
      parent: _controller,
      curve: Curves.easeInOut,
    ));
    
    if (widget.isActive) {
      _controller.repeat(reverse: true);
    }
  }

  @override
  void didUpdateWidget(StatusIndicator oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (widget.isActive && !oldWidget.isActive) {
      _controller.repeat(reverse: true);
    } else if (!widget.isActive && oldWidget.isActive) {
      _controller.stop();
      _controller.value = 0.5;
    }
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: _animation,
      builder: (context, child) {
        return Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            Container(
              width: 8,
              height: 8,
              decoration: BoxDecoration(
                color: widget.statusColor.withOpacity(widget.isActive ? _animation.value : 0.5),
                shape: BoxShape.circle,
                boxShadow: widget.isActive ? [
                  BoxShadow(
                    color: widget.statusColor.withOpacity(0.3 * _animation.value),
                    blurRadius: 4,
                    spreadRadius: 1,
                  ),
                ] : null,
              ),
            ),
            const SizedBox(width: 6),
            Text(
              widget.status,
              style: TextStyle(
                color: AppColors.softWhite,
                fontSize: 11,
                fontWeight: FontWeight.w500,
              ),
            ),
          ],
        );
      },
    );
  }
}

class CompactButton extends StatefulWidget {
  final String text;
  final VoidCallback? onPressed;
  final Color? backgroundColor;
  final Color? textColor;
  final IconData? icon;

  const CompactButton({
    super.key,
    required this.text,
    this.onPressed,
    this.backgroundColor,
    this.textColor,
    this.icon,
  });

  @override
  State<CompactButton> createState() => _CompactButtonState();
}

class _CompactButtonState extends State<CompactButton>
    with SingleTickerProviderStateMixin {
  late AnimationController _controller;
  late Animation<double> _scaleAnimation;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      duration: const Duration(milliseconds: 100),
      vsync: this,
    );
    _scaleAnimation = Tween<double>(
      begin: 1.0,
      end: 0.95,
    ).animate(_controller);
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTapDown: (_) => _controller.forward(),
      onTapUp: (_) => _controller.reverse(),
      onTapCancel: () => _controller.reverse(),
      onTap: widget.onPressed,
      child: AnimatedBuilder(
        animation: _scaleAnimation,
        builder: (context, child) {
          return Transform.scale(
            scale: _scaleAnimation.value,
            child: Container(
              padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 6),
              decoration: BoxDecoration(
                gradient: LinearGradient(
                  begin: Alignment.topCenter,
                  end: Alignment.bottomCenter,
                  colors: [
                    widget.backgroundColor ?? AppColors.primaryBlue,
                    (widget.backgroundColor ?? AppColors.primaryBlue).withOpacity(0.8),
                  ],
                ),
                borderRadius: BorderRadius.circular(8),
                boxShadow: [
                  BoxShadow(
                    color: (widget.backgroundColor ?? AppColors.primaryBlue).withOpacity(0.3),
                    blurRadius: 4,
                    offset: const Offset(0, 2),
                  ),
                ],
              ),
              child: Row(
                mainAxisSize: MainAxisSize.min,
                children: [
                  if (widget.icon != null) ...[
                    Icon(
                      widget.icon,
                      color: widget.textColor ?? AppColors.softWhite,
                      size: 14,
                    ),
                    const SizedBox(width: 4),
                  ],
                  Text(
                    widget.text,
                    style: TextStyle(
                      color: widget.textColor ?? AppColors.softWhite,
                      fontSize: 11,
                      fontWeight: FontWeight.w600,
                    ),
                  ),
                ],
              ),
            ),
          );
        },
      ),
    );
  }
}