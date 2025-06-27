import RPi.GPIO as GPIO

class EmergencyButton:
    def __init__(self, pin):
        self.pin = pin
        GPIO.setup(pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)

    def esta_presionado(self):
        return GPIO.input(self.pin) == GPIO.LOW
