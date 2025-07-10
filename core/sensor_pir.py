import RPi.GPIO as GPIO
import time
import datetime
import signal
import sys

# Configuración del pin
PIR_PIN = 22

# Variables globales
detections = 0
last_detection = None

def setup_gpio():
    """Configura el GPIO para el sensor PIR"""
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(PIR_PIN, GPIO.IN)
    print(f"Sensor PIR configurado en pin {PIR_PIN}")
    print("Esperando que el sensor se estabilice...")
    time.sleep(2)  # Tiempo de estabilización

def motion_detected(channel):
    """Función callback para cuando se detecta movimiento"""
    global detections, last_detection
    
    current_time = datetime.datetime.now()
    detections += 1
    last_detection = current_time
    
    print(f"[{current_time.strftime('%H:%M:%S')}] ¡MOVIMIENTO DETECTADO! (#{detections})")
    
    # Aquí puedes agregar acciones adicionales:
    # - Enviar notificación
    # - Activar alarma
    # - Encender luces
    # - Guardar en base de datos

def signal_handler(sig, frame):
    """Maneja la señal de interrupción para limpiar GPIO"""
    print("\nCerrando programa...")
    GPIO.cleanup()
    sys.exit(0)

def main():
    """Función principal"""
    print("=== SISTEMA DE DETECCIÓN PIR ===")
    print("Presiona Ctrl+C para salir")
    
    # Configurar manejo de señales
    signal.signal(signal.SIGINT, signal_handler)
    
    try:
        # Configurar GPIO
        setup_gpio()
        
        # Configurar interrupción para detección de movimiento
        GPIO.add_event_detect(PIR_PIN, GPIO.RISING, callback=motion_detected, bouncetime=2000)
        
        print("Sistema activo - Monitoreando movimiento...")
        
        # Bucle principal
        while True:
            time.sleep(1)
            
            # Mostrar estadísticas cada 30 segundos
            if int(time.time()) % 30 == 0:
                print(f"Detecciones totales: {detections}")
                if last_detection:
                    time_since = datetime.datetime.now() - last_detection
                    print(f"Última detección: hace {time_since.seconds} segundos")
                
    except Exception as e:
        print(f"Error: {e}")
    finally:
        GPIO.cleanup()

if __name__ == "__main__":
    main()