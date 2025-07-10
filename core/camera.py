import numpy as np
from picamera import PiCamera
from PIL import Image, ImageDraw, ImageFont
import time
import io
from datetime import datetime

class CameraDetectorLite:
    def __init__(self, resolution=(640, 480), framerate=30):
        """
        Detector ligero sin OpenCV
        
        Args:
            resolution: Tupla con resolución (width, height)
            framerate: FPS de captura
        """
        self.camera = PiCamera()
        self.camera.resolution = resolution
        self.camera.framerate = framerate
        
        # Variables para el procesamiento
        self.frame_count = 0
        self.is_running = False
        self.detection_callback = None
        
        # Parámetros de preprocesamiento para CNN
        self.target_size = (224, 224)
        
        # Permitir que la cámara se caliente
        time.sleep(2)
        print("Cámara inicializada correctamente")
    
    def capture_array(self):
        """
        Captura un frame como array numpy
        
        Returns:
            numpy.ndarray: Frame RGB
        """
        stream = io.BytesIO()
        self.camera.capture(stream, format='rgb')
        stream.seek(0)
        
        # Convertir a array numpy
        frame = np.frombuffer(stream.getvalue(), dtype=np.uint8)
        frame = frame.reshape((self.camera.resolution[1], self.camera.resolution[0], 3))
        
        return frame
    
    def preprocess_frame(self, frame):
        """
        Preprocesa el frame para CNN usando PIL
        
        Args:
            frame: Frame numpy array (RGB)
            
        Returns:
            numpy.ndarray: Frame preprocesado
        """
        # Convertir numpy array a PIL Image
        pil_image = Image.fromarray(frame)
        
        # Redimensionar
        resized = pil_image.resize(self.target_size, Image.LANCZOS)
        
        # Convertir de vuelta a numpy
        resized_array = np.array(resized)
        
        # Normalizar (0-255 a 0-1)
        normalized = resized_array.astype(np.float32) / 255.0
        
        # Expandir dimensiones para batch
        batch_frame = np.expand_dims(normalized, axis=0)
        
        return batch_frame
    
    def simple_motion_detection(self, frame1, frame2, threshold=30):
        """
        Detección simple de movimiento sin OpenCV
        
        Args:
            frame1: Frame anterior
            frame2: Frame actual
            threshold: Umbral de diferencia
            
        Returns:
            list: Lista de regiones con movimiento
        """
        if frame1 is None:
            return []
        
        # Convertir a escala de grises (promedio RGB)
        gray1 = np.mean(frame1, axis=2)
        gray2 = np.mean(frame2, axis=2)
        
        # Calcular diferencia
        diff = np.abs(gray2 - gray1)
        
        # Encontrar píxeles que superan el umbral
        motion_mask = diff > threshold
        
        # Encontrar regiones de movimiento (simplificado)
        detections = []
        h, w = motion_mask.shape
        
        # Dividir imagen en bloques y detectar movimiento
        block_size = 40
        for y in range(0, h - block_size, block_size):
            for x in range(0, w - block_size, block_size):
                block = motion_mask[y:y+block_size, x:x+block_size]
                motion_ratio = np.sum(block) / (block_size * block_size)
                
                if motion_ratio > 0.3:  # Si más del 30% del bloque tiene movimiento
                    detections.append([x, y, block_size, block_size, motion_ratio, 0])
        
        return detections
    
    def color_detection(self, frame, color_range=None):
        """
        Detección simple por color
        
        Args:
            frame: Frame RGB
            color_range: Diccionario con rangos de color
            
        Returns:
            list: Lista de detecciones por color
        """
        if color_range is None:
            # Ejemplo: detectar objetos rojos
            color_range = {
                'r_min': 100, 'r_max': 255,
                'g_min': 0, 'g_max': 100,
                'b_min': 0, 'b_max': 100
            }
        
        # Crear máscara de color
        mask = (
            (frame[:, :, 0] >= color_range['r_min']) & 
            (frame[:, :, 0] <= color_range['r_max']) &
            (frame[:, :, 1] >= color_range['g_min']) & 
            (frame[:, :, 1] <= color_range['g_max']) &
            (frame[:, :, 2] >= color_range['b_min']) & 
            (frame[:, :, 2] <= color_range['b_max'])
        )
        
        # Encontrar regiones con el color objetivo
        detections = []
        h, w = mask.shape
        block_size = 30
        
        for y in range(0, h - block_size, block_size):
            for x in range(0, w - block_size, block_size):
                block = mask[y:y+block_size, x:x+block_size]
                color_ratio = np.sum(block) / (block_size * block_size)
                
                if color_ratio > 0.4:  # Si más del 40% del bloque tiene el color
                    detections.append([x, y, block_size, block_size, color_ratio, 1])
        
        return detections
    
    def detect_objects_placeholder(self, preprocessed_frame, original_frame):
        """
        Función placeholder para detección
        Combina detección de movimiento y color
        
        Args:
            preprocessed_frame: Frame preprocesado para CNN
            original_frame: Frame original
            
        Returns:
            list: Lista de detecciones
        """
        detections = []
        
        # Detección de movimiento (necesita frame anterior)
        if hasattr(self, 'previous_frame'):
            motion_detections = self.simple_motion_detection(
                self.previous_frame, original_frame
            )
            detections.extend(motion_detections)
        
        # Detección por color
        color_detections = self.color_detection(original_frame)
        detections.extend(color_detections)
        
        # Guardar frame actual para próxima iteración
        self.previous_frame = original_frame.copy()
        
        return detections
    
    def draw_detections_pil(self, frame, detections):
        """
        Dibuja detecciones usando PIL
        
        Args:
            frame: Frame numpy array
            detections: Lista de detecciones
            
        Returns:
            PIL.Image: Imagen con detecciones
        """
        # Convertir a PIL Image
        pil_image = Image.fromarray(frame)
        draw = ImageDraw.Draw(pil_image)
        
        # Intentar cargar fuente, usar default si no está disponible
        try:
            font = ImageFont.load_default()
        except:
            font = None
        
        colors = [(0, 255, 0), (255, 0, 0), (0, 0, 255), (255, 255, 0)]
        
        for i, detection in enumerate(detections):
            x, y, w, h, confidence, class_id = detection
            
            # Color basado en clase
            color = colors[class_id % len(colors)]
            
            # Dibujar rectángulo
            draw.rectangle([x, y, x + w, y + h], outline=color, width=2)
            
            # Dibujar texto
            label = f"Obj {class_id}: {confidence:.2f}"
            if font:
                draw.text((x, y - 15), label, fill=color, font=font)
            else:
                draw.text((x, y - 15), label, fill=color)
        
        return pil_image
    
    def capture_and_detect_continuous(self, save_detections=False):
        """
        Captura y detección continua
        
        Args:
            save_detections: Si guardar imágenes con detecciones
        """
        print("Iniciando captura continua...")
        self.is_running = True
        
        try:
            while self.is_running:
                # Capturar frame
                frame = self.capture_array()
                self.frame_count += 1
                
                # Preprocesar para CNN
                preprocessed = self.preprocess_frame(frame)
                
                # Detectar objetos
                detections = self.detect_objects_placeholder(preprocessed, frame)
                
                # Mostrar información
                if detections:
                    print(f"Frame {self.frame_count}: {len(detections)} objetos detectados")
                    
                    # Dibujar detecciones
                    image_with_detections = self.draw_detections_pil(frame, detections)
                    
                    # Guardar si se especifica
                    if save_detections:
                        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                        image_with_detections.save(f"detection_{timestamp}.jpg")
                
                # Callback personalizado
                if self.detection_callback:
                    self.detection_callback(detections, frame)
                
                # Pausa pequeña para no sobrecargar
                time.sleep(0.1)
                
        except KeyboardInterrupt:
            print("\nDeteniendo captura...")
        finally:
            self.cleanup()
    
    def capture_single_detection(self):
        """
        Captura un solo frame y detecta
        
        Returns:
            tuple: (frame, detections, preprocessed_frame)
        """
        frame = self.capture_array()
        preprocessed = self.preprocess_frame(frame)
        detections = self.detect_objects_placeholder(preprocessed, frame)
        
        return frame, detections, preprocessed
    
    def save_training_data(self, num_samples=100, folder="training_data"):
        """
        Captura imágenes para entrenar CNN
        
        Args:
            num_samples: Número de imágenes a capturar
            folder: Carpeta donde guardar
        """
        import os
        os.makedirs(folder, exist_ok=True)
        
        print(f"Capturando {num_samples} imágenes para entrenamiento...")
        
        for i in range(num_samples):
            frame = self.capture_array()
            
            # Guardar frame original
            pil_image = Image.fromarray(frame)
            pil_image.save(f"{folder}/sample_{i:04d}.jpg")
            
            # Guardar versión preprocesada
            preprocessed = self.preprocess_frame(frame)
            prep_image = Image.fromarray((preprocessed[0] * 255).astype(np.uint8))
            prep_image.save(f"{folder}/preprocessed_{i:04d}.jpg")
            
            print(f"Capturada imagen {i+1}/{num_samples}")
            time.sleep(0.5)
        
        print(f"Datos de entrenamiento guardados en {folder}/")
    
    def set_detection_callback(self, callback_function):
        """
        Establece callback para procesar detecciones
        
        Args:
            callback_function: Función que recibe (detections, frame)
        """
        self.detection_callback = callback_function
    
    def load_cnn_model(self, model_path):
        """
        Placeholder para cargar modelo CNN
        
        Args:
            model_path: Ruta al modelo
        """
        print(f"Listo para cargar modelo desde: {model_path}")
        # Aquí cargarías tu modelo cuando esté listo
    
    def stop_capture(self):
        """Detiene la captura"""
        self.is_running = False
    
    def cleanup(self):
        """Limpia recursos"""
        self.camera.close()
        print("Cámara cerrada correctamente")

# Ejemplo de uso
def main():
    # Crear detector ligero
    detector = CameraDetectorLite(resolution=(640, 480))
    
    # Función callback de ejemplo
    def detection_callback(detections, frame):
        if detections:
            for i, det in enumerate(detections):
                x, y, w, h, conf, cls = det
                print(f"  Detección {i}: pos=({x},{y}), confianza={conf:.2f}")
    
    # Configurar callback
    detector.set_detection_callback(detection_callback)
    
    print("Opciones disponibles:")
    print("1. Captura continua con detección")
    print("2. Captura única")
    print("3. Generar datos de entrenamiento")
    
    opcion = input("Selecciona opción (1-3): ")
    
    if opcion == "1":
        detector.capture_and_detect_continuous(save_detections=True)
    elif opcion == "2":
        frame, detections, preprocessed = detector.capture_single_detection()
        print(f"Detectados {len(detections)} objetos")
        
        # Mostrar detecciones
        if detections:
            image_with_detections = detector.draw_detections_pil(frame, detections)
            image_with_detections.save("single_detection.jpg")
            print("Imagen guardada como single_detection.jpg")
    elif opcion == "3":
        num_samples = int(input("¿Cuántas imágenes capturar? (default 50): ") or "50")
        detector.save_training_data(num_samples)
    
    detector.cleanup()

if __name__ == "__main__":
    main()