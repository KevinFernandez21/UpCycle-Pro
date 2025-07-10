from gpiozero import MotionSensor
from signal import pause
import time

# Inicializar el sensor PIR en el pin GPIO 22
pir = MotionSensor(22)

def motion_detected():
    print("¡Movimiento detectado!")

def no_motion_detected():
    print("No hay movimiento")

# Opción 1: Usando callbacks (recomendado)
print("Iniciando detección de movimiento con callbacks...")
pir.when_motion = motion_detected
pir.when_no_motion = no_motion_detected

# Mantener el programa corriendo
try:
    pause()
except KeyboardInterrupt:
    print("\nPrograma terminado")

# Opción 2: Usando bucle while (tu código original corregido)
"""
print("Iniciando detección de movimiento con bucle...")
try:
    while True:
        pir.wait_for_motion()
        print("¡Te moviste!")
        pir.wait_for_no_motion()
        print("Ya no hay movimiento")
        time.sleep(0.1)  # Pequeña pausa para evitar lecturas excesivas
except KeyboardInterrupt:
    print("\nPrograma terminado")
"""