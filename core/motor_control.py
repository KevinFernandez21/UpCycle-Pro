import RPi.GPIO as GPIO
import time

# Configuración del GPIO
GPIO.setmode(GPIO.BCM)  # Usar numeración BCM
GPIO.setwarnings(False)

# Pin donde está conectado el transistor 2N222
MOTOR_PIN = 27

# Configurar el pin como salida
GPIO.setup(MOTOR_PIN, GPIO.OUT)

def motor_on():
    """Enciende el motor"""
    GPIO.output(MOTOR_PIN, GPIO.HIGH)
    print("Motor encendido")

def motor_off():
    """Apaga el motor"""
    GPIO.output(MOTOR_PIN, GPIO.LOW)
    print("Motor apagado")

def motor_pulse(duration=1):
    """Pulso del motor por un tiempo determinado"""
    motor_on()
    time.sleep(duration)
    motor_off()

def motor_pwm(duty_cycle=50, frequency=1000):
    """Control PWM del motor para velocidad variable"""
    pwm = GPIO.PWM(MOTOR_PIN, frequency)
    pwm.start(duty_cycle)
    return pwm

# Ejemplo de uso
try:
    print("=== Control de Motor con 2N222 ===")
    
    # Encender motor por 2 segundos
    print("1. Encendiendo motor por 2 segundos...")
    motor_pulse(2)
    time.sleep(1)
    
    # Pulsos cortos
    print("2. Pulsos cortos...")
    for i in range(5):
        motor_pulse(0.5)
        time.sleep(0.5)
    
    # Control PWM (velocidad variable)
    print("3. Control PWM - Velocidad variable...")
    pwm = motor_pwm(30, 1000)  # 30% duty cycle, 1kHz
    time.sleep(3)
    
    # Aumentar velocidad gradualmente
    print("4. Aumentando velocidad...")
    for speed in range(30, 101, 10):
        pwm.ChangeDutyCycle(speed)
        print(f"   Velocidad: {speed}%")
        time.sleep(1)
    
    # Detener PWM
    pwm.stop()
    
    print("5. Programa terminado")

except KeyboardInterrupt:
    print("\nPrograma interrumpido por el usuario")

finally:
    # Limpiar configuración GPIO
    GPIO.cleanup()
    print("GPIO limpiado")