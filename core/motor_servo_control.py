# SPDX-FileCopyrightText: 2024 Servo Controller
# SPDX-License-Identifier: MIT

import time
import board
from adafruit_motor import servo
from adafruit_pca9685 import PCA9685

class ServoController:
    def __init__(self, pca_channel, servo_type="180", min_pulse=750, max_pulse=2250):
        """
        Inicializa el controlador de servo
        
        Args:
            pca_channel: Canal del PCA9685 (0-15)
            servo_type: Tipo de servo - "180", "270", "360" o "continuo"
            min_pulse: Pulso mínimo en microsegundos
            max_pulse: Pulso máximo en microsegundos
        """
        self.i2c = board.I2C()
        self.pca = PCA9685(self.i2c)
        self.pca.frequency = 50
        
        self.servo_type = servo_type
        self.channel = pca_channel
        self.home_position = 0  # Posición inicial/home
        
        # Configurar rangos según tipo de servo
        if servo_type == "180":
            self.max_angle = 180
            self.servo_obj = servo.Servo(self.pca.channels[pca_channel], 
                                       min_pulse=min_pulse, max_pulse=max_pulse)
        elif servo_type == "270":
            self.max_angle = 270
            self.servo_obj = servo.Servo(self.pca.channels[pca_channel], 
                                       min_pulse=min_pulse, max_pulse=max_pulse,
                                       actuation_range=270)
        elif servo_type == "360" or servo_type == "continuo":
            self.max_angle = 360
            self.servo_obj = servo.ContinuousServo(self.pca.channels[pca_channel])
        
        # Ir a posición inicial
        self.go_to_home()
    
    def go_to_home(self):
        """Mueve el servo a la posición inicial"""
        if self.servo_type in ["180", "270"]:
            self.servo_obj.angle = self.home_position
            print(f"Servo en posición HOME: {self.home_position}°")
        elif self.servo_type in ["360", "continuo"]:
            self.servo_obj.throttle = 0.0
            print("Servo continuo detenido")
        time.sleep(0.5)
    
    def move_to_angle(self, angle):
        """Mueve el servo a un ángulo específico"""
        if self.servo_type in ["180", "270"]:
            if 0 <= angle <= self.max_angle:
                self.servo_obj.angle = angle
                print(f"Moviendo servo a: {angle}°")
                time.sleep(0.1)
            else:
                print(f"Ángulo fuera de rango. Máximo: {self.max_angle}°")
        else:
            print("Función no disponible para servos continuos")
    
    def sweep_movement(self, speed_delay=0.03):
        """Ejecuta movimiento completo y regresa a HOME"""
        print(f"Iniciando movimiento completo del servo {self.servo_type}°...")
        
        if self.servo_type in ["180", "270"]:
            # Movimiento hacia adelante
            for angle in range(0, self.max_angle + 1, 2):
                self.servo_obj.angle = angle
                time.sleep(speed_delay)
            
            print(f"Alcanzado máximo: {self.max_angle}°")
            time.sleep(0.5)
            
            # Regreso a HOME
            for angle in range(self.max_angle, self.home_position - 1, -2):
                self.servo_obj.angle = angle
                time.sleep(speed_delay)
                
            print("Regresado a posición HOME")
            
        elif self.servo_type in ["360", "continuo"]:
            # Para servos continuos, rotar en una dirección por tiempo determinado
            rotation_time = 2.0  # segundos
            
            print("Rotando en sentido horario...")
            self.servo_obj.throttle = 1.0
            time.sleep(rotation_time)
            
            print("Rotando en sentido antihorario...")
            self.servo_obj.throttle = -1.0
            time.sleep(rotation_time)
            
            print("Deteniendo servo...")
            self.servo_obj.throttle = 0.0
    
    def detect_and_activate(self, sensor_input):
        """
        Función que se activa cuando se detecta algo
        
        Args:
            sensor_input: Valor del sensor (True/False o "detectado")
        """
        if sensor_input == "detectado" or sensor_input == True:
            print("¡DETECCIÓN ACTIVADA!")
            self.sweep_movement()
            return True
        return False
    
    def manual_control(self):
        """Control manual del servo"""
        print(f"\n--- Control Manual Servo {self.servo_type}° ---")
        print("Comandos disponibles:")
        print("- 'home': Ir a posición inicial")
        print("- 'sweep': Movimiento completo")
        print("- 'detectado': Simular detección")
        print("- ángulo (0-{}): Ir a ángulo específico".format(self.max_angle))
        print("- 'salir': Terminar control")
        
        while True:
            comando = input("\nIngresa comando: ").lower().strip()
            
            if comando == 'salir':
                break
            elif comando == 'home':
                self.go_to_home()
            elif comando == 'sweep':
                self.sweep_movement()
            elif comando == 'detectado':
                self.detect_and_activate("detectado")
            elif comando.isdigit():
                angle = int(comando)
                self.move_to_angle(angle)
            else:
                print("Comando no reconocido")
    
    def cleanup(self):
        """Limpia recursos"""
        self.go_to_home()
        self.pca.deinit()
        print("Recursos liberados")

# Ejemplo de uso
if __name__ == "__main__":
    # CONFIGURACIÓN - Modifica estos valores según tu servo
    SERVO_TYPE = "180"  # Cambia por "180", "270", "360" o "continuo"
    PCA_CHANNEL = 7     # Canal del PCA9685 (0-15)
    
    # Crear controlador
    print(f"Inicializando servo {SERVO_TYPE}° en canal {PCA_CHANNEL}")
    controller = ServoController(PCA_CHANNEL, SERVO_TYPE)
    
    try:
        # Ejemplo de detección automática
        print("\n--- Prueba de Detección Automática ---")
        time.sleep(2)
        
        # Simular detección
        controller.detect_and_activate("detectado")
        
        time.sleep(2)
        
        # Control manual
        controller.manual_control()
        
    except KeyboardInterrupt:
        print("\nInterrupción por teclado")
    finally:
        controller.cleanup()

# Ejemplo de uso con diferentes configuraciones:
"""
# Para servo de 180°:
controller = ServoController(7, "180")

# Para servo de 270°:
controller = ServoController(7, "270")

# Para servo continuo/360°:
controller = ServoController(7, "360")

# Con pulsos personalizados:
controller = ServoController(7, "180", min_pulse=500, max_pulse=2500)
"""