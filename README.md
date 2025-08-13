# UpCycle Pro - Sistema de Clasificación Automática de Materiales

## 📋 Descripción

UpCycle Pro es un sistema inteligente de clasificación automática de materiales reciclables que utiliza visión por computadora con redes neuronales convolucionales (CNN) para identificar y separar materiales como vidrio, plástico y metal.

## 🏗️ Arquitectura del Sistema

```
UpCycle-Pro/
├── 🤖 ai_client/CNN/          # Modelos CNN entrenados (.keras, .h5)
├── 🔧 backend/api/            # FastAPI backend
│   ├── main.py               # Servidor principal FastAPI
│   ├── routes/               # Rutas de la API
│   └── requirements.txt      # Dependencias Python
├── 📱 frontend/              # App móvil Flutter
│   ├── lib/
│   │   ├── screens/          # Pantallas de la aplicación
│   │   ├── services/         # Servicios de API y configuración
│   │   └── widgets/          # Widgets personalizados
│   └── pubspec.yaml          # Dependencias Flutter
├── 🔩 microcontrolador/      # Código para ESP32
│   ├── UpCyclePRO_microcontrolador/  # Firmware principal
│   └── tests/                # Tests para hardware
├── 📊 dashboard.py           # Dashboard Streamlit
├── 🚀 start_api.py          # Script para iniciar FastAPI
├── 🚀 run_dashboard.py      # Script para iniciar Dashboard
├── 🚀 run_all.py            # Script para iniciar todo el sistema
└── 🚀 start_system.bat     # Script de Windows
```

## 🚀 Componentes Principales

### 1. Backend (FastAPI + CNN)
- **Clasificación de imágenes** con modelo CNN entrenado
- **API REST** para comunicación con frontend y microcontrolador
- **Endpoints especializados** para control de hardware
- **Separación automática** basada en confianza del modelo

### 2. Frontend (Flutter)
- **Interfaz moderna** con animaciones y tema personalizado
- **Cámara integrada** para captura y clasificación en tiempo real
- **Monitoreo del sistema** con datos de sensores en vivo
- **Controles manuales** para operación del sistema

### 3. Microcontrolador (ESP32)
- **Sensores PIR** para detección de movimiento
- **Sensor de peso HX711** para medición de masa
- **Control de motores** para transportador
- **Servo motores** para separación de materiales

## 🛠️ Tecnologías Utilizadas

### Backend
- **FastAPI** - Framework web moderno y rápido
- **TensorFlow** - Machine learning y CNN
- **OpenCV** - Procesamiento de imágenes
- **Python 3.9+** - Lenguaje principal

### Frontend
- **Flutter** - Framework multiplataforma
- **Dart** - Lenguaje de programación
- **Camera Plugin** - Integración con cámara
- **HTTP** - Comunicación con API

### Hardware
- **ESP32** - Microcontrolador principal
- **Arduino IDE** - Desarrollo del firmware
- **Sensores PIR** - Detección de movimiento
- **HX711** - Lectura de peso
- **Servo motores** - Control de separación

## 📦 Instalación y Configuración

### Prerrequisitos
```bash
- Python 3.9+
- Flutter SDK
- Arduino IDE
- Node.js (opcional para herramientas)
```

### 🚀 Inicio Rápido

**Opción 1: Sistema Completo**
```bash
# Windows
start_system.bat

# Linux/Mac
python run_all.py
```

**Opción 2: Servicios Individuales**

Backend FastAPI:
```bash
python start_api.py
# Disponible en: http://localhost:8000
# Docs: http://localhost:8000/docs
```

Dashboard Streamlit:
```bash
pip install -r dashboard_requirements.txt
python run_dashboard.py
# Disponible en: http://localhost:8501
```

Frontend Flutter:
```bash
cd frontend
flutter pub get
flutter run
```

### 📦 Instalación Manual

Backend:
```bash
cd backend/api
pip install -r requirements.txt
```

Dashboard:
```bash
pip install -r dashboard_requirements.txt
```

### Frontend Setup
```bash
cd frontend
flutter pub get
flutter run
```

### Configuración de Red
1. Actualizar IP del backend en `config.yaml`
2. Configurar IP del ESP32 en la red local
3. Verificar conectividad entre componentes

## 🎯 Funcionalidades Principales

### Clasificación Inteligente
- **Detección automática** de materiales con CNN
- **Confianza del modelo** > 70% para separación automática
- **Soporte para lotes** de múltiples imágenes
- **Almacenamiento de estadísticas** de clasificación

### Control del Sistema
- **Activación automática** por sensores PIR y peso
- **Control manual** desde la aplicación móvil
- **Monitoreo en tiempo real** del estado del sistema
- **Separación precisa** con servo posicionado por material

### Interfaz de Usuario
- **Pantalla de inicio** con estado del sistema
- **Vista de cámara** con clasificación en vivo
- **Monitor del sistema** con datos de sensores
- **Configuración** personalizable

## 📊 Flujo de Operación

1. **Detección**: Sensores PIR detectan presencia de objeto
2. **Pesado**: Sensor HX711 confirma peso significativo (>0.1kg)
3. **Captura**: Cámara toma imagen del material
4. **Clasificación**: CNN procesa imagen y determina material
5. **Separación**: Servo se posiciona según el material detectado
6. **Transporte**: Motor transportador mueve el objeto al contenedor correcto

## 🔧 Configuración de Materiales

### Posiciones del Servo
```yaml
glass: 45°      # Vidrio
plastic: 90°    # Plástico (posición neutral)
metal: 135°     # Metal
```

### Umbrales del Sistema
```yaml
confidence_threshold: 0.7    # Confianza mínima para separación
weight_threshold: 0.1        # Peso mínimo en kg
update_interval: 1000        # Actualización de sensores en ms
```

## 🌐 API Endpoints

### ℹ️ Información y Estado
- `GET /` - Información general del sistema
- `GET /health` - Estado de salud de la API
- `GET /available_models` - Lista de modelos CNN disponibles
- `GET /model_info` - Información del modelo actual

### 🤖 Gestión de Modelos
- `POST /load_model` - Cargar modelo CNN por defecto
- `POST /load_model?model_name=best_model.keras` - Cargar modelo específico

### 📸 Clasificación  
- `POST /predict` - Clasificar imagen individual
- `POST /predict_batch` - Clasificar múltiples imágenes

### 🔧 Microcontrolador (ESP32)
- `GET /microcontroller/status` - Estado del sistema
- `POST /microcontroller/system/activate` - Activar sistema
- `POST /microcontroller/system/deactivate` - Desactivar sistema
- `POST /microcontroller/motor/control` - Control de motores
- `GET /microcontroller/sensor/data` - Datos de sensores

## 📊 Dashboard de Streamlit

El dashboard incluye:
- **Estado del Sistema**: Métricas en tiempo real
- **Gestión de Modelos**: Cargar y cambiar modelos CNN
- **Predicción de Imágenes**: Interface para clasificar materiales
- **Estadísticas**: Gráficos y análisis de datos

### Características del Dashboard:
- 🏠 **Panel Principal**: Estado general del sistema
- 🤖 **Modelos CNN**: Gestión y carga de modelos
- 📸 **Predicción**: Cargar imágenes para clasificar  
- 📊 **Estadísticas**: Métricas y gráficos de rendimiento

## 🔍 Monitoreo y Logs

- **Logs detallados** de clasificaciones y operaciones
- **Métricas de rendimiento** del modelo CNN
- **Estado en tiempo real** de todos los componentes
- **Almacenamiento de imágenes** clasificadas (opcional)

## 🚨 Manejo de Errores

- **Recuperación automática** en caso de desconexión
- **Fallback a modo manual** si CNN falla
- **Notificaciones en tiempo real** de errores
- **Logs detallados** para debugging

## 📈 Futuras Mejoras

- [ ] **Base de datos** para almacenamiento persistente
- [ ] **Dashboard web** para administración
- [ ] **Más materiales** en el modelo CNN
- [ ] **Integración IoT** con plataformas cloud
- [ ] **Análisis predictivo** de patrones de reciclaje
- [ ] **API de estadísticas** y reportes

## 🤝 Contribución

Para contribuir al proyecto:

1. Fork el repositorio
2. Crear una rama para tu feature
3. Commit tus cambios
4. Push a la rama
5. Crear un Pull Request

## 📄 Licencia

Este proyecto está bajo la Licencia MIT - ver el archivo [LICENSE](LICENSE) para detalles.

## 📞 Soporte

Para soporte técnico o preguntas:
- Crear un [Issue](https://github.com/usuario/UpCycle-Pro/issues)
- Contactar al equipo de desarrollo

---

**UpCycle Pro** - Tecnología inteligente para un futuro más sostenible 🌱