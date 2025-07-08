import RPi.GPIO as GPIO
import time

# Configuración de los pines GPIO
LED1_PIN = 17  # Primer LED en GPIO 17
LED2_PIN = 27  # Segundo LED en GPIO 27

# Configurar el modo de numeración de pines
GPIO.setmode(GPIO.BCM)

# Configurar los pines como salida
GPIO.setup(LED1_PIN, GPIO.OUT)
GPIO.setup(LED2_PIN, GPIO.OUT)

try:
    # Encender ambos LEDs
    print("Encendiendo ambos LEDs...")
    GPIO.output(LED1_PIN, GPIO.HIGH)
    GPIO.output(LED2_PIN, GPIO.HIGH)
    time.sleep(2)
    
    # Apagar ambos LEDs
    print("Apagando ambos LEDs...")
    GPIO.output(LED1_PIN, GPIO.LOW)
    GPIO.output(LED2_PIN, GPIO.LOW)
    time.sleep(1)
    
    # Parpadeo alternado
    print("Parpadeo alternado...")
    for i in range(10):
        GPIO.output(LED1_PIN, GPIO.HIGH)  # Encender LED1
        GPIO.output(LED2_PIN, GPIO.LOW)   # Apagar LED2
        time.sleep(0.5)
        GPIO.output(LED1_PIN, GPIO.LOW)   # Apagar LED1
        GPIO.output(LED2_PIN, GPIO.HIGH)  # Encender LED2
        time.sleep(0.5)
    
    # Parpadeo simultáneo
    print("Parpadeo simultáneo...")
    for i in range(10):
        GPIO.output(LED1_PIN, GPIO.HIGH)  # Encender ambos
        GPIO.output(LED2_PIN, GPIO.HIGH)
        time.sleep(0.3)
        GPIO.output(LED1_PIN, GPIO.LOW)   # Apagar ambos
        GPIO.output(LED2_PIN, GPIO.LOW)
        time.sleep(0.3)
        
except KeyboardInterrupt:
    print("Programa interrumpido por el usuario")
    
finally:
    # Limpiar la configuración GPIO
    GPIO.cleanup()
    print("GPIO limpiado y programa terminado")