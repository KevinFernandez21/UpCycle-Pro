import RPi.GPIO as GPIO

class PIRSensor:
    def __init__(self, pin):
        self.pin = pin
        GPIO.setup(pin, GPIO.IN)

    def detecta_movimiento(self):
        return GPIO.input(self.pin)
