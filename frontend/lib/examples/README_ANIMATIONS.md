# 🎨 Guía Completa de Animaciones - UpCycle Pro

Esta guía contiene todos los widgets animados disponibles, sus parámetros y ejemplos de uso.

## 📋 Tabla de Contenidos

1. [PulsingDot](#pulsingdot) - Indicadores pulsantes
2. [AnimatedProgressBar](#animatedprogressbar) - Barras de progreso animadas  
3. [AnimatedCard](#animatedcard) - Cards interactivas
4. [FloatingButton](#floatingbutton) - Botones flotantes
5. [RotatingIcon](#rotatingicon) - Iconos rotatorios
6. [SlideInAnimation](#slideinanimation) - Animaciones de entrada

---

## 🔴 PulsingDot

**Descripción:** Crea un punto que pulsa continuamente, ideal para indicadores de estado.

### Parámetros:
```dart
PulsingDot({
  required Color color,        // Color del punto
  double size = 16,           // Tamaño del punto
  Duration duration = const Duration(seconds: 2), // Velocidad del pulso
})
```

### Ejemplos de Uso:

#### ✅ Estado Online (Verde, Lento)
```dart
PulsingDot(
  color: Colors.green,
  size: 16,
  duration: Duration(seconds: 2),
)
```

#### ❌ Estado Error (Rojo, Rápido)
```dart
PulsingDot(
  color: Colors.red,
  size: 16,
  duration: Duration(milliseconds: 500),
)
```

#### 🔵 Indicador Personalizado
```dart
PulsingDot(
  color: Color(0xFF00aaff),
  size: 24,
  duration: Duration(milliseconds: 1000),
)
```

### Casos de Uso:
- ✅ Indicadores de conexión (Online/Offline)
- ✅ Estado de sensores (OK/Error)
- ✅ Indicadores de transmisión en vivo
- ✅ Alertas y notificaciones

---

## 📊 AnimatedProgressBar

**Descripción:** Barra de progreso que se anima suavemente al cambiar de valor.

### Parámetros:
```dart
AnimatedProgressBar({
  required String label,       // Etiqueta de la barra
  required String value,       // Valor mostrado (ej: "75%")
  required double progress,    // Progreso (0.0 - 1.0)
  required Color color,        // Color de la barra
  Duration duration = const Duration(milliseconds: 1500), // Duración de la animación
})
```

### Ejemplos de Uso:

#### 🖥️ CPU Usage
```dart
AnimatedProgressBar(
  label: 'CPU Usage',
  value: '75%',
  progress: 0.75,
  color: Colors.red,
  duration: Duration(milliseconds: 1500),
)
```

#### 💾 Memory Usage
```dart
AnimatedProgressBar(
  label: 'Memory',
  value: '8.5 GB',
  progress: 0.85,
  color: Color(0xFF4ecdc4),
  duration: Duration(milliseconds: 2000),
)
```

#### 🌡️ Temperature
```dart
AnimatedProgressBar(
  label: 'Temperature',
  value: '42°C',
  progress: 0.60,
  color: Color(0xFFff9a9e),
  duration: Duration(milliseconds: 2500),
)
```

### Casos de Uso:
- ✅ Monitoreo de sistema (CPU, RAM, Temperatura)
- ✅ Progreso de procesos
- ✅ Niveles de sensores
- ✅ Configuración de parámetros

---

## 🃏 AnimatedCard

**Descripción:** Card que responde al toque con animaciones de escala y elevación.

### Parámetros:
```dart
AnimatedCard({
  required Widget child,       // Contenido de la card
  EdgeInsetsGeometry? padding, // Padding interno
  double? width,              // Ancho específico
  VoidCallback? onTap,        // Función al tocar
})
```

### Ejemplos de Uso:

#### 📊 Card de Sistema
```dart
AnimatedCard(
  onTap: () => print('Sistema presionado'),
  child: Column(
    children: [
      Icon(Icons.computer, color: Colors.blue, size: 32),
      Text('Sistema', style: TextStyle(color: Colors.white)),
      Text('CPU: 75%', style: TextStyle(color: Colors.grey)),
    ],
  ),
)
```

#### 📈 Card de Estadísticas
```dart
AnimatedCard(
  width: double.infinity,
  child: Column(
    crossAxisAlignment: CrossAxisAlignment.start,
    children: [
      Text('[PC] Sistema', style: TextStyle(color: Color(0xFF00aaff))),
      SizedBox(height: 16),
      AnimatedProgressBar(
        label: 'CPU',
        value: '65%',
        progress: 0.65,
        color: Colors.red,
      ),
    ],
  ),
)
```

### Casos de Uso:
- ✅ Cards de información del sistema
- ✅ Controles interactivos
- ✅ Paneles de configuración
- ✅ Elementos clicables

---

## 🎈 FloatingButton

**Descripción:** Botón que flota suavemente arriba y abajo de forma continua.

### Parámetros:
```dart
FloatingButton({
  required Widget child,       // Contenido del botón
  VoidCallback? onPressed,     // Función al presionar
  Duration duration = const Duration(seconds: 3), // Duración del ciclo de flotación
})
```

### Ejemplos de Uso:

#### 🎛️ Botón de Control
```dart
FloatingButton(
  duration: Duration(seconds: 3),
  onPressed: () => controlAction(),
  child: Container(
    height: 50,
    decoration: BoxDecoration(
      gradient: LinearGradient(
        colors: [Color(0xFF667eea), Color(0xFF764ba2)],
      ),
      borderRadius: BorderRadius.circular(8),
    ),
    child: Center(
      child: Text('CONTROL', style: TextStyle(color: Colors.white)),
    ),
  ),
)
```

#### 📷 Botón de Cámara
```dart
FloatingButton(
  duration: Duration(seconds: 4),
  onPressed: () => takePhoto(),
  child: Container(
    padding: EdgeInsets.all(12),
    decoration: BoxDecoration(
      color: Colors.black.withOpacity(0.5),
      border: Border.all(color: Colors.white.withOpacity(0.3)),
      borderRadius: BorderRadius.circular(8),
    ),
    child: Text('[*] FOTO', style: TextStyle(color: Colors.white)),
  ),
)
```

### Casos de Uso:
- ✅ Botones de control de cámara
- ✅ Controles de sistema
- ✅ Botones de acción importantes
- ✅ Elementos que requieren atención

---

## 🌟 RotatingIcon

**Descripción:** Icono que rota continuamente, ideal para indicar procesos activos.

### Parámetros:
```dart
RotatingIcon({
  required IconData icon,      // Icono a rotar
  Color color = Colors.white,  // Color del icono
  double size = 24,           // Tamaño del icono
  Duration duration = const Duration(seconds: 4), // Velocidad de rotación
})
```

### Ejemplos de Uso:

#### ⚙️ Settings/Configuración
```dart
RotatingIcon(
  icon: Icons.settings,
  color: Colors.grey,
  size: 32,
  duration: Duration(seconds: 4),
)
```

#### 🔄 Refresh/Actualizar
```dart
RotatingIcon(
  icon: Icons.refresh,
  color: Color(0xFF00aaff),
  size: 24,
  duration: Duration(seconds: 2),
)
```

#### 📹 Cámara Activa
```dart
RotatingIcon(
  icon: Icons.videocam,
  color: Colors.green,
  size: 32,
  duration: Duration(seconds: 6),
)
```

#### 🧠 IA Processing
```dart
RotatingIcon(
  icon: Icons.psychology,
  color: Color(0xFF00aaff),
  size: 24,
  duration: Duration(seconds: 3),
)
```

### Casos de Uso:
- ✅ Indicar procesos en ejecución
- ✅ Cámara en funcionamiento
- ✅ Cargando/Procesando
- ✅ Configuración activa

---

## ✨ SlideInAnimation

**Descripción:** Anima la entrada de un widget deslizándolo desde una dirección específica.

### Parámetros:
```dart
SlideInAnimation({
  required Widget child,       // Widget a animar
  Duration duration = const Duration(milliseconds: 800), // Duración de la animación
  Duration delay = Duration.zero, // Retraso antes de iniciar
  Offset begin = const Offset(0, 0.5), // Posición inicial
})
```

### Direcciones de Entrada:

#### ⬅️ Desde la Izquierda
```dart
SlideInAnimation(
  begin: Offset(-0.5, 0),
  duration: Duration(milliseconds: 800),
  child: Text('Desde la izquierda'),
)
```

#### ➡️ Desde la Derecha
```dart
SlideInAnimation(
  begin: Offset(0.5, 0),
  duration: Duration(milliseconds: 800),
  child: Text('Desde la derecha'),
)
```

#### ⬆️ Desde Abajo
```dart
SlideInAnimation(
  begin: Offset(0, 0.5),
  duration: Duration(milliseconds: 800),
  child: Text('Desde abajo'),
)
```

#### ⬇️ Desde Arriba
```dart
SlideInAnimation(
  begin: Offset(0, -0.5),
  duration: Duration(milliseconds: 800),
  child: Text('Desde arriba'),
)
```

### Animaciones Escalonadas:
```dart
Column(
  children: [
    SlideInAnimation(
      begin: Offset(0, 0.3),
      duration: Duration(milliseconds: 800),
      delay: Duration(milliseconds: 0),
      child: Card1(),
    ),
    SlideInAnimation(
      begin: Offset(0, 0.3),
      duration: Duration(milliseconds: 800),
      delay: Duration(milliseconds: 200),
      child: Card2(),
    ),
    SlideInAnimation(
      begin: Offset(0, 0.3),
      duration: Duration(milliseconds: 800),
      delay: Duration(milliseconds: 400),
      child: Card3(),
    ),
  ],
)
```

### Casos de Uso:
- ✅ Entrada de pantallas
- ✅ Aparición de cards
- ✅ Secuencias de animación
- ✅ Revelar contenido

---

## 🚀 Ejemplos Combinados

### 🖥️ Sistema Completo
```dart
SlideInAnimation(
  begin: Offset(0, -0.3),
  child: AnimatedCard(
    child: Column(
      children: [
        // Header con estado
        Row(
          children: [
            PulsingDot(
              color: isOnline ? Colors.green : Colors.red,
              duration: isOnline ? Duration(seconds: 2) : Duration(milliseconds: 500),
            ),
            SizedBox(width: 12),
            Text('Sistema ${isOnline ? "Online" : "Offline"}'),
          ],
        ),
        
        // Progreso del sistema
        AnimatedProgressBar(
          label: 'CPU Usage',
          value: '${cpuUsage.toInt()}%',
          progress: cpuUsage,
          color: cpuUsage > 0.8 ? Colors.red : Colors.green,
        ),
        
        // Controles
        FloatingButton(
          child: ActionButton('Refresh'),
          onPressed: () => refreshSystem(),
        ),
      ],
    ),
  ),
)
```

### 📹 Control de Cámara
```dart
AnimatedCard(
  child: Column(
    children: [
      // Indicador de transmisión
      Row(
        children: [
          PulsingDot(
            color: Colors.green,
            duration: Duration(milliseconds: 800),
          ),
          SizedBox(width: 12),
          Text('EN VIVO - 1080p'),
        ],
      ),
      
      // Vista de cámara
      Container(
        child: Center(
          child: RotatingIcon(
            icon: Icons.videocam,
            duration: Duration(seconds: 6),
          ),
        ),
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
    ],
  ),
)
```

### 🔍 Grid de Sensores
```dart
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
)

Widget SensorWidget(String label, bool isOk) {
  return AnimatedCard(
    child: Column(
      children: [
        Text(label),
        Row(
          children: [
            PulsingDot(
              color: isOk ? Colors.green : Colors.red,
              duration: isOk ? Duration(seconds: 2) : Duration(milliseconds: 500),
            ),
            Text(isOk ? 'OK' : 'ERR'),
          ],
        ),
      ],
    ),
  );
}
```

## 🎯 Mejores Prácticas

### ⏱️ Timing y Duración
- **PulsingDot**: 2s para estados normales, 500ms para errores
- **AnimatedProgressBar**: 1.5-2.5s para cambios suaves
- **FloatingButton**: 3-6s para movimiento sutil
- **RotatingIcon**: 3-8s para rotación lenta
- **SlideInAnimation**: 800-1200ms para entradas suaves

### 🎨 Colores Recomendados
- **Verde**: Estados OK, Online, Éxito
- **Rojo**: Errores, Offline, Peligro
- **Azul (#00aaff)**: Información, Procesos activos
- **Naranja**: Advertencias, Pausas
- **Gris**: Elementos inactivos

### 📱 Rendimiento
- Usar `SingleTickerProviderStateMixin` cuando sea posible
- Disponer correctamente los controllers en `dispose()`
- Evitar animaciones simultáneas excesivas
- Usar delays para animaciones escalonadas

### 🎪 Secuencias de Animación
```dart
// Ejemplo de secuencia escalonada
Column(
  children: [
    SlideInAnimation(
      delay: Duration(milliseconds: 0),
      child: Header(),
    ),
    SlideInAnimation(
      delay: Duration(milliseconds: 200),
      child: SystemCard(),
    ),
    SlideInAnimation(
      delay: Duration(milliseconds: 400),
      child: Controls(),
    ),
  ],
)
```

---

## 📁 Archivos de Ejemplo

- `animation_examples.dart` - Ejemplos básicos de cada componente
- `advanced_examples.dart` - Casos de uso reales y combinaciones
- `animated_widgets.dart` - Implementación de todos los widgets

## 🔧 Instalación

1. Copia `animated_widgets.dart` a tu carpeta `lib/widgets/`
2. Importa en tu archivo: `import '../widgets/animated_widgets.dart';`
3. Usa los widgets según los ejemplos de esta guía

¡Disfruta creando interfaces animadas espectaculares! 🎉