import RPi.GPIO as GPIO
import time
import statistics

class HX711:
    def __init__(self, dout_pin, sck_pin):
        self.dout_pin = dout_pin
        self.sck_pin = sck_pin
        self.reference_unit = 1
        self.offset = 0
        
        # Configurar GPIO
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self.dout_pin, GPIO.IN)
        GPIO.setup(self.sck_pin, GPIO.OUT)
        GPIO.output(self.sck_pin, False)
        
        # Esperar a que el sensor esté listo
        self.wait_ready()
        
    def wait_ready(self):
        """Espera hasta que el sensor esté listo"""
        while not GPIO.input(self.dout_pin):
            time.sleep(0.01)
        while GPIO.input(self.dout_pin):
            time.sleep(0.01)
    
    def read_raw(self):
        """Lee el valor crudo del sensor"""
        self.wait_ready()
        
        data = 0
        # Leer 24 bits
        for i in range(24):
            GPIO.output(self.sck_pin, True)
            data = data << 1
            GPIO.output(self.sck_pin, False)
            if GPIO.input(self.dout_pin):
                data += 1
        
        # Pulso adicional para el gain (128 por defecto)
        GPIO.output(self.sck_pin, True)
        GPIO.output(self.sck_pin, False)
        
        # Convertir a valor con signo (complemento a 2)
        if data & 0x800000:
            data -= 0x1000000
        
        return data
    
    def read_average(self, samples=10):
        """Lee el promedio de múltiples muestras"""
        values = []
        for _ in range(samples):
            values.append(self.read_raw())
            time.sleep(0.01)
        return statistics.mean(values)
    
    def tare(self, samples=10):
        """Establece el punto cero (tara)"""
        self.offset = self.read_average(samples)
        print(f"Tara establecida: {self.offset}")
    
    def set_reference_unit(self, reference_unit):
        """Establece la unidad de referencia para calibración"""
        self.reference_unit = reference_unit
    
    def get_weight(self, samples=10):
        """Obtiene el peso calibrado"""
        value = self.read_average(samples) - self.offset
        return value / self.reference_unit
    
    def power_down(self):
        """Apaga el sensor"""
        GPIO.output(self.sck_pin, False)
        GPIO.output(self.sck_pin, True)
    
    def power_up(self):
        """Enciende el sensor"""
        GPIO.output(self.sck_pin, False)
    
    def cleanup(self):
        """Limpia los pines GPIO"""
        GPIO.cleanup()

# Configuración de pines
DT_PIN = 10  # Pin GPIO 10 (pin físico 19)
SCK_PIN = 9  # Pin GPIO 9 (pin físico 21)

def calibrate_sensor(hx711):
    """Función para calibrar el sensor"""
    print("=== CALIBRACIÓN DEL SENSOR ===")
    print("1. Retira todo peso de la balanza y presiona Enter")
    input()
    
    print("Estableciendo tara...")
    hx711.tare(20)
    
    print("2. Coloca un peso conocido en la balanza")
    known_weight = float(input("Ingresa el peso en gramos: "))
    
    print("Leyendo valor con peso...")
    raw_value = hx711.read_average(20) - hx711.offset
    
    reference_unit = raw_value / known_weight
    hx711.set_reference_unit(reference_unit)
    
    print(f"Unidad de referencia calculada: {reference_unit}")
    print(f"Peso actual: {hx711.get_weight()} g")
    
    return reference_unit

def main():
    print("Iniciando HX711 en Raspberry Pi...")
    print(f"DT Pin: GPIO {DT_PIN}")
    print(f"SCK Pin: GPIO {SCK_PIN}")
    
    try:
        # Crear instancia del sensor
        hx711 = HX711(DT_PIN, SCK_PIN)
        
        # Opción de calibración
        calibrate = input("¿Quieres calibrar el sensor? (s/n): ").lower()
        
        if calibrate == 's':
            reference_unit = calibrate_sensor(hx711)
        else:
            # Valores por defecto (necesitarás ajustar según tu sensor)
            print("Usando valores por defecto...")
            hx711.tare()
            # Ajusta este valor según tu calibración
            hx711.set_reference_unit(1)  # Cambia este valor
        
        print("\n=== LECTURAS DEL SENSOR ===")
        print("Presiona Ctrl+C para salir")
        
        while True:
            try:
                # Leer valor crudo
                raw_value = hx711.read_raw()
                
                # Leer valor con tara
                tared_value = raw_value - hx711.offset
                
                # Leer peso calibrado
                weight = hx711.get_weight()
                
                print(f"Crudo: {raw_value:8d} | Con tara: {tared_value:8d} | Peso: {weight:8.2f} g")
                
                time.sleep(0.5)
                
            except KeyboardInterrupt:
                print("\nSaliendo...")
                break
                
    except Exception as e:
        print(f"Error: {e}")
        
    finally:
        hx711.cleanup()
        print("GPIO limpiado")

if __name__ == "__main__":
    main()