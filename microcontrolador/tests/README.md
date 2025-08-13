# 🧪 Tests UpCycle Pro - Microcontrolador

Esta carpeta contiene tests individuales para cada componente del sistema UpCycle Pro, así como un test de integración completa.

## 📋 Estructura de Tests

```
tests/
├── test_pir_sensors/           # Test sensores PIR individuales
├── test_pca9685_servos/        # Test controlador de servos
├── test_motor_2n222/          # Test motor con transistor
├── test_oled_display/         # Test display OLED
├── test_i2c_scanner/          # Scanner diagnóstico I2C
├── test_integration_complete/ # Test completo integrado
└── README.md                  # Este archivo
```

## 🎯 Orden Recomendado de Testing

### 1. **I2C Scanner** - `test_i2c_scanner`
**Propósito:** Diagnosticar comunicación I2C y detectar dispositivos conectados.

**Cuándo usar:**
- Primera verificación del sistema
- Problemas de comunicación I2C
- Verificar direcciones de dispositivos

**Dispositivos esperados:**
- `0x40`: PCA9685 (Controlador de servos)
- `0x3C` o `0x3D`: OLED SSD1306

### 2. **PIR Sensors** - `test_pir_sensors`
**Propósito:** Verificar funcionamiento de los 3 sensores PIR de detección.

**Componentes:**
- PIR1 (VIDRIO): GPIO26
- PIR2 (PLASTICO): GPIO27
- PIR3 (METAL): GPIO14

**Características:**
- Detección individual de cada sensor
- Contador de detecciones
- LED indicador de actividad
- Reporte periódico de estado

### 3. **Motor 2N222** - `test_motor_2n222`
**Propósito:** Probar control del motor de banda transportadora.

**Componentes:**
- Motor 12V controlado por transistor 2N222
- Control PWM desde GPIO25

**Características:**
- Control de velocidad 0-100%
- Demo automático con diferentes velocidades
- Prueba completa de arranque/frenado
- Monitoreo simulado de corriente

### 4. **OLED Display** - `test_oled_display`
**Propósito:** Verificar funcionamiento del display OLED.

**Componentes:**
- Display SSD1306 128x32 en I2C
- Alimentación 3.3V

**Características:**
- Pantallas del sistema (IDLE, ESPERANDO, PROCESANDO)
- Prueba de gráficos y texto
- Demo automático de interfaces
- Detección de resolución incorrecta

### 5. **PCA9685 Servos** - `test_pca9685_servos`
**Propósito:** Probar controlador PCA9685 y los 3 servos.

**Componentes:**
- PCA9685 en I2C con 3 servos
- Servo VIDRIO: Canal 0
- Servo PLASTICO: Canal 1
- Servo METAL: Canal 2

**Características:**
- Prueba individual de cada servo
- Demo automático rotativo
- Control manual por comandos
- Verificación de comunicación I2C

### 6. **Integration Complete** - `test_integration_complete`
**Propósito:** Probar todo el sistema funcionando integrado.

**Componentes:** Todos los anteriores trabajando juntos.

**Características:**
- Simulación completa del proceso de clasificación
- Máquina de estados del sistema
- Control por comandos seriales
- Estadísticas de procesamiento
- Manejo de errores

## 🔌 Conexiones Físicas

### Alimentación
```
ESP32:      5V USB
PCA9685:    5V
Servos:     5V (máx 1.5A total)
PIR:        5V
Motor:      12V (a través de 2N222)
OLED:       3.3V ⚠️ IMPORTANTE
```

### Pines GPIO
```
PIR Sensors:
- PIR1 (VIDRIO):    GPIO26
- PIR2 (PLASTICO):  GPIO27
- PIR3 (METAL):     GPIO14

Motor:
- Control 2N222:    GPIO25

I2C (compartido):
- SDA:              GPIO21
- SCL:              GPIO22

LED Indicador:
- LED onboard:      GPIO2
```

### Circuito Motor 2N222
```
ESP32 GPIO25 → R220Ω → Base 2N222
Colector 2N222 → Motor 12V (+)
Emisor 2N222 → GND común
Motor 12V (-) → GND común
```

## 🛠️ Instalación de Librerías

Instalar en Arduino IDE:
```
1. Adafruit_PWMServoDriver (para PCA9685)
2. Adafruit_SSD1306 (para OLED)
3. Adafruit_GFX (dependencia del OLED)
```

## 📋 Procedimiento de Testing Completo

### Fase 1: Verificación Básica
1. **I2C Scanner**: Verificar que PCA9685 y OLED se detectan
2. **PIR Sensors**: Verificar detección de movimiento en los 3 sensores
3. **Motor**: Verificar control de velocidad del motor

### Fase 2: Verificación Avanzada  
4. **OLED Display**: Verificar todas las pantallas del sistema
5. **PCA9685 Servos**: Verificar movimiento de los 3 servos

### Fase 3: Integración
6. **Integration Complete**: Verificar funcionamiento completo del sistema

## 🎮 Comandos Principales

### Tests Individuales
```
PIR Sensors: Automático + reportes periódicos
Motor 2N222: '0'-'4' velocidades, 'd' demo, 't' prueba completa
OLED: '1'-'3' materiales, 'i' ícono, 'd' demo, 't' gráficos
PCA9685: '1'-'3' servos, 'a' auto, 'r' reset, 's' estado
I2C Scanner: 's' escanear, 'a' auto, 't' test, 'v' velocidad
```

### Test Integración
```
'1' / '2' / '3': Clasificar VIDRIO/PLASTICO/METAL
'stop': Detener sistema
'status': Estado completo
'test': Prueba automática completa
'stats': Estadísticas de procesamiento
```

## ❌ Resolución de Problemas

### No se detectan dispositivos I2C
- Verificar conexiones SDA/SCL (GPIO21/22)
- Verificar alimentación (3.3V OLED, 5V PCA9685)
- Agregar resistencias pull-up 4.7kΩ

### PIR no detecta
- Verificar alimentación 5V
- Esperar estabilización (2-5 minutos)
- Verificar conexiones GPIO

### Motor no funciona
- Verificar circuito 2N222
- Verificar alimentación 12V
- Motor debe consumir <500mA

### OLED en blanco
- Verificar voltaje 3.3V (NO 5V)
- Verificar resolución 128x32
- Probar dirección 0x3D

### Servos no responden
- Verificar PCA9685 detectado en I2C
- Verificar alimentación 5V servos
- Verificar conexiones a canales correctos

## 📊 Resultados Esperados

### ✅ Sistema Perfecto
- I2C Scanner: 2 dispositivos (0x40, 0x3C/0x3D)
- PIR: 3 sensores detectando correctamente
- Motor: Control suave de velocidad 0-100%
- OLED: Todas las pantallas funcionando
- Servos: 3 servos moviéndose correctamente
- Integración: Clasificación automática funcional

### ⚠️ Sistema Funcional con Limitaciones
- Algunos componentes no detectados
- Funcionalidad básica disponible
- Continuar con tests individuales

### ❌ Sistema No Funcional
- Sin dispositivos I2C detectados
- Verificar conexiones básicas
- Revisar alimentación

## 🚀 Siguiente Paso

Una vez que todos los tests individuales pasen correctamente y el test de integración funcione, el sistema está listo para:

1. Cargar el código principal `UpCyclePRO_microcontrolador.ino`
2. Usar la librería `SimpleClassifier` 
3. Integrar con el frontend Flutter
4. Implementación del sistema completo

## 📞 Soporte

Si encuentra problemas:
1. Ejecutar I2C Scanner primero
2. Verificar conexiones según este documento
3. Probar tests individuales en orden
4. Revisar alimentación de cada componente
5. Consultar comentarios detallados en cada sketch

¡El sistema UpCycle Pro está diseñado para ser robusto y fácil de diagnosticar! 🎉