# SPDX-FileCopyrightText: 2021 ladyada for Adafruit Industries
# SPDX-License-Identifier: MIT

import time
import board
from adafruit_motor import servo
from adafruit_pca9685 import PCA9685

# CONFIGURACIÓN - Modifica estos valores según tu servo
SERVO_DEGREES = 180  # Cambia por 270, 360, etc.
PCA_CHANNEL = 7      # Canal del PCA9685

# Configuración I2C (igual que tu código original)
i2c = board.I2C()  # uses board.SCL and board.SDA
pca = PCA9685(i2c)
pca.frequency = 50

# Configuración del servo (igual que tu código original)
servo7 = servo.Servo(pca.channels[PCA_CHANNEL])

def go_to_home():
    """Mueve el servo a posición inicial (0 grados)"""
    servo7.angle = 0
    print("Servo en posición HOME (0°)")
    time.sleep(0.5)

def detection_activated():
    """
    Función que se ejecuta cuando hay detección
    Mueve el servo completamente según SERVO_DEGREES y regresa a HOME
    """
    print("¡DETECCIÓN ACTIVADA!")
    print(f"Moviendo servo de 0° a {SERVO_DEGREES}°...")
    
    # Movimiento hacia adelante (igual que tu código original)
    for i in range(SERVO_DEGREES):
        servo7.angle = i
        time.sleep(0.03)
    
    print(f"Alcanzado máximo: {SERVO_DEGREES}°")
    time.sleep(0.5)
    
    # Movimiento de regreso (igual que tu código original)  
    print("Regresando a posición HOME...")
    for i in range(SERVO_DEGREES):
        servo7.angle = SERVO_DEGREES - i
        time.sleep(0.03)
    
    print("Movimiento completo terminado")

def move_to_angle(angle):
    """Mueve el servo a un ángulo específico"""
    if 0 <= angle <= SERVO_DEGREES:
        servo7.angle = angle
        print(f"Servo movido a: {angle}°")
        time.sleep(0.1)
    else:
        print(f"Ángulo fuera de rango. Máximo: {SERVO_DEGREES}°")

def fractional_movement():
    """Movimiento fraccional (igual que tu código original)"""
    print("Ejecutando movimiento fraccional...")
    fraction = 0.0
    while fraction < 1.0:
        servo7.fraction = fraction
        fraction += 0.01
        time.sleep(0.03)
    print("Movimiento fraccional completado")

def test_original_code():
    """Ejecuta exactamente tu código original"""
    print("Ejecutando código original...")
    
    # Tu código original
    for i in range(SERVO_DEGREES):
        servo7.angle = i
        time.sleep(0.03)
    for i in range(SERVO_DEGREES):
        servo7.angle = SERVO_DEGREES - i
        time.sleep(0.03)

    # Movimiento fraccional original
    fraction = 0.0
    while fraction < 1.0:
        servo7.fraction = fraction
        fraction += 0.01
        time.sleep(0.03)

def cleanup():
    """Limpia recursos (igual que tu código original)"""
    pca.deinit()
    print("Recursos liberados")

# Función principal para simular detección
def check_detection():
    """
    Aquí conectarías tu sensor o lógica de detección
    Por ahora simula la detección
    """
    # Simulación - reemplaza esto con tu lógica real
    detection_signal = "detectado"  # Esto vendría de tu sensor
    
    if detection_signal == "detectado":
        detection_activated()
        return True
    return False

# Programa principal
if __name__ == "__main__":
    print(f"Servo de {SERVO_DEGREES}° inicializado en canal {PCA_CHANNEL}")
    
    try:
        # Ir a posición inicial
        go_to_home()
        
        # Menú simple
        print("\n--- CONTROL DE SERVO ---")
        print("1. Presiona 'd' para simular detección")
        print("2. Presiona 't' para ejecutar código original")
        print("3. Presiona 'f' para movimiento fraccional")
        print("4. Ingresa un número (0-{}) para ir a ese ángulo".format(SERVO_DEGREES))
        print("5. Presiona 'q' para salir")
        
        while True:
            comando = input("\nComando: ").lower().strip()
            
            if comando == 'q':
                break
            elif comando == 'd':
                # Simular detección
                print("Simulando detección...")
                detection_activated()
            elif comando == 't':
                test_original_code()
            elif comando == 'f':
                fractional_movement()
            elif comando == 'h':
                go_to_home()
            elif comando.isdigit():
                angle = int(comando)
                move_to_angle(angle)
            else:
                print("Comando no reconocido")
                
    except KeyboardInterrupt:
        print("\nInterrupción por teclado")
    finally:
        cleanup()

# Ejemplo de uso en tu código principal:
"""
# Para usar en tu proyecto principal:

# 1. Importa las funciones necesarias
from motor_servo_control import detection_activated, go_to_home

# 2. En tu lógica principal, cuando detectes algo:
if sensor_reading == "detectado":
    detection_activated()

# 3. Para volver a posición inicial:
go_to_home()
"""