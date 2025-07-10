import cv2
import numpy as np
from picamera import PiCamera
from picamera.array import PiRGBArray
import time
import threading
from datetime import datetime

class CameraDetector:
    def __init__(self, resolution=(640, 480), framerate=30):
        """
        Inicializa la cámara para detección de objetos
        
        Args:
            resolution: Tupla con resolución (width, height)
            framerate: FPS de captura
        """
        self.camera = PiCamera()
        self.camera.resolution = resolution
        self.camera.framerate = framerate
        self.rawCapture = PiRGBArray(self.camera, size=resolution)
        
        # Variables para el procesamiento
        self.frame_count = 0
        self.is_running = False
        self.detection_callback = None
        
        # Parámetros de preprocesamiento para CNN
        self.target_size = (224, 224)  # Tamaño común para CNNs
        self.mean_subtraction = [123.68, 116.78, 103.94]  # ImageNet means
        
        # Permitir que la cámara se caliente
        time.sleep(2)
    
    def preprocess_frame(self, frame):
        """
        Preprocesa el frame para CNN
        
        Args:
            frame: Frame de OpenCV (BGR)
            
        Returns:
            numpy.ndarray: Frame preprocesado
        """
        # Convertir BGR a RGB (para CNN)
        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        
        # Redimensionar al tamaño objetivo
        resized = cv2.resize(frame_rgb, self.target_size)
        
        # Normalizar valores de pixel (0-255 a 0-1)
        normalized = resized.astype(np.float32) / 255.0
        
        # Sustracción de media (opcional, depende del modelo)
        # normalized = normalized - np.array(self.mean_subtraction) / 255.0
        
        # Expandir dimensiones para batch (1, height, width, channels)
        batch_frame = np.expand_dims(normalized, axis=0)
        
        return batch_frame
    
    def detect_objects_placeholder(self, preprocessed_frame):
        """
        Función placeholder para detección de objetos
        Aquí irá tu modelo CNN entrenado
        
        Args:
            preprocessed_frame: Frame preprocesado
            
        Returns:
            list: Lista de detecciones [x, y, w, h, confidence, class]
        """
        # PLACEHOLDER: Aquí cargarías y usarías tu modelo CNN
        # Ejemplo con TensorFlow/Keras:
        # predictions = self.model.predict(preprocessed_frame)
        # detections = self.process_predictions(predictions)
        
        # Por ahora, detección simple de contornos como ejemplo
        frame_bgr = cv2.cvtColor(preprocessed_frame[0], cv2.COLOR_RGB2BGR)
        frame_uint8 = (frame_bgr * 255).astype(np.uint8)
        
        gray = cv2.cvtColor(frame_uint8, cv2.COLOR_BGR2GRAY)
        blurred = cv2.GaussianBlur(gray, (5, 5), 0)
        edges = cv2.Canny(blurred, 50, 150)
        
        contours, _ = cv2.findContours(edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        
        detections = []
        for contour in contours:
            if cv2.contourArea(contour) > 500:  # Filtrar contornos pequeños
                x, y, w, h = cv2.boundingRect(contour)
                # Escalar coordenadas de vuelta al tamaño original
                confidence = 0.8  # Placeholder
                class_id = 0  # Placeholder
                detections.append([x, y, w, h, confidence, class_id])
        
        return detections
    
    def draw_detections(self, frame, detections):
        """
        Dibuja las detecciones en el frame
        
        Args:
            frame: Frame original
            detections: Lista de detecciones
            
        Returns:
            numpy.ndarray: Frame con detecciones dibujadas
        """
        for detection in detections:
            x, y, w, h, confidence, class_id = detection
            
            # Dibujar rectángulo
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
            
            # Dibujar texto
            label = f"Object {class_id}: {confidence:.2f}"
            cv2.putText(frame, label, (x, y - 10), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
        
        return frame
    
    def capture_and_detect(self, save_detections=False):
        """
        Captura frames y realiza detección en tiempo real
        
        Args:
            save_detections: Si guardar frames con detecciones
        """
        print("Iniciando captura y detección...")
        self.is_running = True
        
        try:
            for frame in self.camera.capture_continuous(self.rawCapture, 
                                                       format="bgr", 
                                                       use_video_port=True):
                if not self.is_running:
                    break
                
                # Obtener frame actual
                current_frame = frame.array
                self.frame_count += 1
                
                # Preprocesar para CNN
                preprocessed = self.preprocess_frame(current_frame)
                
                # Detectar objetos
                detections = self.detect_objects_placeholder(preprocessed)
                
                # Dibujar detecciones
                frame_with_detections = self.draw_detections(current_frame.copy(), detections)
                
                # Mostrar frame
                cv2.imshow("Detección de Objetos - Raspberry Pi", frame_with_detections)
                
                # Guardar si se especifica
                if save_detections and detections:
                    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                    cv2.imwrite(f"detection_{timestamp}.jpg", frame_with_detections)
                
                # Callback personalizado (opcional)
                if self.detection_callback:
                    self.detection_callback(detections, current_frame)
                
                # Limpiar stream para próxima captura
                self.rawCapture.truncate(0)
                
                # Salir con 'q'
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break
                
        except Exception as e:
            print(f"Error durante la captura: {e}")
        finally:
            self.cleanup()
    
    def capture_single_frame(self):
        """
        Captura un solo frame y retorna detecciones
        
        Returns:
            tuple: (frame, detections, preprocessed_frame)
        """
        self.rawCapture.truncate(0)
        self.camera.capture(self.rawCapture, format="bgr")
        
        frame = self.rawCapture.array
        preprocessed = self.preprocess_frame(frame)
        detections = self.detect_objects_placeholder(preprocessed)
        
        return frame, detections, preprocessed
    
    def set_detection_callback(self, callback_function):
        """
        Establece una función callback para procesar detecciones
        
        Args:
            callback_function: Función que recibe (detections, frame)
        """
        self.detection_callback = callback_function
    
    def load_cnn_model(self, model_path):
        """
        Carga un modelo CNN entrenado
        
        Args:
            model_path: Ruta al modelo
        """
        # Implementar carga del modelo según framework usado
        # TensorFlow/Keras:
        # import tensorflow as tf
        # self.model = tf.keras.models.load_model(model_path)
        
        # PyTorch:
        # import torch
        # self.model = torch.load(model_path)
        
        print(f"Modelo CNN cargado desde: {model_path}")
    
    def stop_capture(self):
        """Detiene la captura"""
        self.is_running = False
    
    def cleanup(self):
        """Limpia recursos"""
        cv2.destroyAllWindows()
        self.camera.close()
        print("Recursos liberados")

# Ejemplo de uso
def main():
    # Crear detector
    detector = CameraDetector(resolution=(640, 480), framerate=30)
    
    # Función callback de ejemplo
    def detection_callback(detections, frame):
        if detections:
            print(f"Detectados {len(detections)} objetos")
            for i, det in enumerate(detections):
                x, y, w, h, conf, cls = det
                print(f"  Objeto {i}: pos=({x},{y}), size=({w},{h}), conf={conf:.2f}")
    
    # Configurar callback
    detector.set_detection_callback(detection_callback)
    
    # Opción 1: Captura continua
    try:
        detector.capture_and_detect(save_detections=True)
    except KeyboardInterrupt:
        print("\nDeteniendo captura...")
        detector.stop_capture()
    
    # Opción 2: Captura única
    # frame, detections, preprocessed = detector.capture_single_frame()
    # print(f"Capturado frame con {len(detections)} detecciones")

if __name__ == "__main__":
    main()