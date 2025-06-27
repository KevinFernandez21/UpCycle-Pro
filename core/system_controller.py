import time
from core.motor_control import MotorController
from core.sensor_pir import PIRSensor
from core.sensor_hx711 import Balanza
from core.emergency_button import EmergencyButton
from ai_client.aws_cnn import predecir_material
from ai_client.aws_lstm import enviar_peso
from ui_tft.main import Pantalla

class SystemController:
    def __init__(self):
        self.pir = PIRSensor(pin=17)
        self.motor = MotorController()
        self.balanza = Balanza()
        self.emergency = EmergencyButton(pin=27)
        self.pantalla = Pantalla()
        self.activo = True

    def run(self):
        self.pantalla.mostrar_estado("Sistema activo")
        self.motor.iniciar_banda()

        while True:
            if self.emergency.esta_presionado():
                self.motor.detener_todo()
                self.pantalla.mostrar_estado("Emergencia activada")
                break

            if self.pir.detecta_movimiento():
                self.motor.detener_banda()
                self.pantalla.mostrar_estado("Detectando material...")

                clase = predecir_material()
                self.motor.activar_led_y_servo(clase)

                peso = self.balanza.leer_peso()
                enviar_peso(clase, peso)

                time.sleep(1)
                self.motor.reiniciar_servo(clase)
                self.motor.iniciar_banda()
                self.pantalla.mostrar_estado("Esperando material...")

            time.sleep(0.2)
