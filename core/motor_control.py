from adafruit_pca9685 import PCA9685
from board import SCL, SDA
import busio
import RPi.GPIO as GPIO
import time

LED_PINS = {"plastico": 5, "carton": 6, "vidrio": 13}
MOTOR_PIN = 18

class MotorController:
    def __init__(self):
        self.i2c = busio.I2C(SCL, SDA)
        self.pca = PCA9685(self.i2c)
        self.pca.frequency = 50
        GPIO.setmode(GPIO.BCM)
        for pin in LED_PINS.values():
            GPIO.setup(pin, GPIO.OUT)
        GPIO.setup(MOTOR_PIN, GPIO.OUT)
        self.motor_pwm = GPIO.PWM(MOTOR_PIN, 100)
        self.motor_pwm.start(0)

    def iniciar_banda(self):
        self.motor_pwm.ChangeDutyCycle(50)

    def detener_banda(self):
        self.motor_pwm.ChangeDutyCycle(0)

    def activar_led_y_servo(self, clase):
        self._encender_led(clase)
        canal = {"plastico": 0, "vidrio": 1, "carton": 2}[clase]
        self.pca.channels[canal].duty_cycle = 0x7FFF

    def reiniciar_servo(self, clase):
        canal = {"plastico": 0, "vidrio": 1, "carton": 2}[clase]
        self.pca.channels[canal].duty_cycle = 0

    def _encender_led(self, clase):
        for tipo, pin in LED_PINS.items():
            GPIO.output(pin, tipo == clase)

    def detener_todo(self):
        self.detener_banda()
        for pin in LED_PINS.values():
            GPIO.output(pin, False)
        for canal in range(3):
            self.pca.channels[canal].duty_cycle = 0
