import subprocess
import numpy as np
from PIL import Image
import time
import os
from datetime import datetime

class CameraDetectorCompatible:
    def __init__(self, resolution=(640, 480)):
        """
        Detector compatible usando subprocess para captura
        
        Args:
            resolution: Tupla con resolución (width, height)
        """
        self.resolution = resolution
        self.width, self.height = resolution
        
        # Variables para el procesamiento
        self.frame_count = 0
        self.is_running = False
        self.detection_callback = None
        self.target_size = (224, 224)
        
        # Verificar que los comandos de cámara están disponibles
        self.camera_command = self._detect_camera_command()
        print(f"Usando comando de cámara: {self.camera_command}")
    
    def _detect_camera_command(self):
        """Detecta qué comando de cámara está disponible"""
        commands = [
            "libcamera-still",
            "raspistill", 
            "fswebcam"
        ]
        
        for cmd in commands:
            try:
                result = subprocess.run([cmd, "--help"], 
                                      capture_output=True, 
                                      text=True, 
                                      timeout=5)
                if result.returncode == 0 or "usage" in result.stderr.lower():
                    return cmd
            except (subprocess.TimeoutExpired, FileNotFoundError):
                continue
        
        raise Exception("No se encontró ningún comando de cámara compatible")
    
    def capture_image_file(self, filename="temp_capture.jpg"):
        """
        Captura una imagen usando subprocess
        
        Args:
            filename: Nombre del archivo temporal
            
        Returns:
            bool: True si la captura fue exitosa
        """
        try:
            if self.camera_command == "libcamera-still":
                cmd = [
                    "libcamera-still",
                    "-o", filename,
                    "--width", str(self.width),
                    "--height", str(self.height),
                    "--timeout", "1000",
                    "--immediate"
                ]
            elif self.camera_command == "raspistill":
                cmd = [
                    "raspistill",
                    "-o", filename,
                    "-w", str(self.width),
                    "-h", str(self.height),
                    "-t", "1000",
                    "-q", "80"
                ]
            else:  # fswebcam
                cmd = [
                    "fswebcam",
                    "-r", f"{self.width}x{self.height}",
                    "--no-banner",
                    filename
                ]
            
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=10)
            
            if result.returncode == 0 and os.path.exists(filename):
                return True
            else:
                print(f"Error en captura: {result.stderr}")
                return False
                
        except Exception as e:
            print(f"Error ejecutando comando de cámara: {e}")
            return False
    
    def load_image_as_array(self, filename):
        """
        Carga imagen como array numpy
        
        Args:
            filename: Nombre del archivo
            
        Returns:
            numpy.ndarray: Imagen como array RGB
        """
        try:
            pil_image = Image.open(filename)
            # Convertir a RGB si es necesario
            if pil_image.mode != 'RGB':
                pil_image = pil_image.convert('RGB')
            
            # Convertir a array numpy
            frame = np.array(pil_image)
            return frame
            
        except Exception as e:
            print(f"Error cargando imagen: {e}")
            return None
    
    def capture_frame(self):
        """
        Captura un frame completo
        
        Returns:
            numpy.ndarray: Frame como array RGB o None si falla
        """
        temp_file = f"temp_frame_{int(time.time())}.jpg"
        
        try:
            # Capturar imagen
            if self.capture_image_file(temp_file):
                # Cargar como array
                frame = self.load_image_as_array(temp_file)
                # Limpiar archivo temporal
                os.remove(temp_file)
                return frame
            else:
                return None
                
        except Exception as e:
            print(f"Error en captura de frame: {e}")
            # Limpiar archivo temporal si existe
            if os.path.exists(temp_file):
                os.remove(temp_file)
            return None
    
    def preprocess_frame(self, frame):
        """
        Preprocesa frame para CNN
        
        Args:
            frame: Frame numpy array (RGB)
            
        Returns:
            numpy.ndarray: Frame preprocesado
        """
        if frame is None:
            return None
        
        # Convertir a PIL para redimensionar
        pil_image = Image.fromarray(frame)
        resized = pil_image.resize(self.target_size, Image.LANCZOS)
        
        # Convertir a array y normalizar
        resized_array = np.array(resized)
        normalized = resized_array.astype(np.float32) / 255.0
        
        # Expandir dimensiones para batch
        batch_frame = np.expand_dims(normalized, axis=0)
        
        return batch_frame
    
    def simple_brightness_detection(self, frame, threshold=100):
        """
        Detección simple basada en brillo
        
        Args:
            frame: Frame RGB
            threshold: Umbral de brillo
            
        Returns:
            list: Lista de detecciones
        """
        if frame is None:
            return []
        
        # Convertir a escala de grises
        gray = np.mean(frame, axis=2)
        
        # Encontrar regiones brillantes
        bright_mask = gray > threshold
        
        # Encontrar regiones conectadas (simplificado)
        detections = []
        h, w = bright_mask.shape
        block_size = 50
        
        for y in range(0, h - block_size, block_size):
            for x in range(0, w - block_size, block_size):
                block = bright_mask[y:y+block_size, x:x+block_size]
                bright_ratio = np.sum(block) / (block_size * block_size)
                
                if bright_ratio > 0.3:  # Si más del 30% es brillante
                    detections.append([x, y, block_size, block_size, bright_ratio, 0])
        
        return detections
    
    def detect_objects_placeholder(self, preprocessed_frame, original_frame):
        """
        Función placeholder para detección
        
        Args:
            preprocessed_frame: Frame preprocesado
            original_frame: Frame original
            
        Returns:
            list: Lista de detecciones
        """
        # Detección simple por brillo
        detections = self.simple_brightness_detection(original_frame)
        
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
        if frame is None:
            return None
        
        pil_image = Image.fromarray(frame)
        
        if detections:
            from PIL import ImageDraw, ImageFont
            draw = ImageDraw.Draw(pil_image)
            
            try:
                font = ImageFont.load_default()
            except:
                font = None
            
            for detection in detections:
                x, y, w, h, confidence, class_id = detection
                
                # Dibujar rectángulo
                draw.rectangle([x, y, x + w, y + h], outline=(0, 255, 0), width=2)
                
                # Dibujar texto
                label = f"Obj: {confidence:.2f}"
                draw.text((x, y - 15), label, fill=(0, 255, 0), font=font)
        
        return pil_image
    
    def capture_and_detect_interval(self, interval=2, max_captures=50, save_detections=True):
        """
        Captura y detecta con intervalos (más estable)
        
        Args:
            interval: Segundos entre capturas
            max_captures: Máximo número de capturas
            save_detections: Si guardar detecciones
        """
        print(f"Iniciando captura cada {interval} segundos...")
        print(f"Máximo {max_captures} capturas")
        
        self.is_running = True
        captures_done = 0
        
        try:
            while self.is_running and captures_done < max_captures:
                print(f"Captura {captures_done + 1}/{max_captures}")
                
                # Capturar frame
                frame = self.capture_frame()
                
                if frame is not None:
                    self.frame_count += 1
                    
                    # Preprocesar
                    preprocessed = self.preprocess_frame(frame)
                    
                    # Detectar
                    detections = self.detect_objects_placeholder(preprocessed, frame)
                    
                    if detections:
                        print(f"  Detectados {len(detections)} objetos")
                        
                        # Dibujar detecciones
                        image_with_detections = self.draw_detections_pil(frame, detections)
                        
                        # Guardar
                        if save_detections and image_with_detections:
                            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                            filename = f"detection_{timestamp}.jpg"
                            image_with_detections.save(filename)
                            print(f"  Guardado: {filename}")
                    else:
                        print("  No se detectaron objetos")
                    
                    # Callback personalizado
                    if self.detection_callback:
                        self.detection_callback(detections, frame)
                else:
                    print("  Error en captura")
                
                captures_done += 1
                
                # Esperar intervalo
                if captures_done < max_captures:
                    print(f"  Esperando {interval} segundos...")
                    time.sleep(interval)
                
        except KeyboardInterrupt:
            print("\nCaptura interrumpida por usuario")
        except Exception as e:
            print(f"Error durante captura: {e}")
        finally:
            self.is_running = False
            print("Captura terminada")
    
    def capture_single_detection(self):
        """
        Captura única con detección
        
        Returns:
            tuple: (frame, detections, preprocessed_frame)
        """
        print("Capturando imagen...")
        
        frame = self.capture_frame()
        if frame is None:
            return None, [], None
        
        preprocessed = self.preprocess_frame(frame)
        detections = self.detect_objects_placeholder(preprocessed, frame)
        
        return frame, detections, preprocessed
    
    def test_camera(self):
        """
        Prueba básica de la cámara
        
        Returns:
            bool: True si la cámara funciona
        """
        print("Probando cámara...")
        
        success = self.capture_image_file("test_camera.jpg")
        
        if success:
            print("✓ Cámara funcionando correctamente")
            print("✓ Imagen de prueba guardada como test_camera.jpg")
            return True
        else:
            print("✗ Error: No se pudo capturar imagen")
            return False
    
    def set_detection_callback(self, callback_function):
        """Establecer callback para detecciones"""
        self.detection_callback = callback_function
    
    def stop_capture(self):
        """Detener captura"""
        self.is_running = False
    
    def cleanup(self):
        """Limpiar recursos"""
        print("Limpieza completada")

# Ejemplo de uso
def main():
    try:
        # Crear detector compatible
        detector = CameraDetectorCompatible(resolution=(640, 480))
        
        # Probar cámara primero
        if not detector.test_camera():
            print("La cámara no funciona. Verifica la conexión y configuración.")
            return
        
        # Callback de ejemplo
        def detection_callback(detections, frame):
            if detections:
                print(f"Callback: {len(detections)} objetos detectados")
        
        detector.set_detection_callback(detection_callback)
        
        print("\nOpciones disponibles:")
        print("1. Captura única")
        print("2. Captura con intervalos")
        print("3. Solo probar cámara")
        
        opcion = input("Selecciona opción (1-3): ")
        
        if opcion == "1":
            frame, detections, preprocessed = detector.capture_single_detection()
            if frame is not None:
                print(f"Detectados {len(detections)} objetos")
                
                if detections:
                    image_with_detections = detector.draw_detections_pil(frame, detections)
                    image_with_detections.save("single_detection.jpg")
                    print("Resultado guardado como single_detection.jpg")
            else:
                print("Error en captura")
        
        elif opcion == "2":
            interval = float(input("Intervalo entre capturas (segundos, default 3): ") or "3")
            max_captures = int(input("Número máximo de capturas (default 10): ") or "10")
            detector.capture_and_detect_interval(interval, max_captures)
        
        elif opcion == "3":
            detector.test_camera()
        
        detector.cleanup()
        
    except Exception as e:
        print(f"Error: {e}")
        print("\nPosibles soluciones:")
        print("1. Verificar que la cámara esté conectada")
        print("2. Ejecutar: sudo raspi-config -> Enable Camera")
        print("3. Instalar: sudo apt install libcamera-apps")
        print("4. Reiniciar el sistema")

if __name__ == "__main__":
    main()