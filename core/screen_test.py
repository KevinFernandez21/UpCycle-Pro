#!/usr/bin/env python3
import time
import spidev
import RPi.GPIO as GPIO
from PIL import Image, ImageDraw, ImageFont
import digitalio
import board
import adafruit_ili9341
from threading import Thread

class TouchDisplay:
    def __init__(self):
        # Configurar display
        self.setup_display()
        
        # Configurar touch
        self.setup_touch()
        
        # Variables para dibujo
        self.drawing = False
        self.last_pos = None
        
        # Crear imagen para dibujar
        self.image = Image.new("RGB", (self.display.width, self.display.height))
        self.draw = ImageDraw.Draw(self.image)
        
        # Limpiar pantalla
        self.clear_screen()
        
    def setup_display(self):
        """Configurar display ILI9341"""
        # Pines del display
        cs_pin = digitalio.DigitalInOut(board.CE0)    # Pin 24 (GPIO 8)
        dc_pin = digitalio.DigitalInOut(board.D24)    # Pin 18 (GPIO 24)
        reset_pin = digitalio.DigitalInOut(board.D25) # Pin 22 (GPIO 25)
        
        # SPI
        spi = board.SPI()
        
        # Crear display
        self.display = adafruit_ili9341.ILI9341(
            spi,
            cs=cs_pin,
            dc=dc_pin,
            rst=reset_pin,
            width=320,
            height=240,
            rotation=90
        )
        
    def setup_touch(self):
        """Configurar touch XPT2046"""
        self.spi_touch = spidev.SpiDev()
        self.spi_touch.open(0, 1)  # SPI0, CE1
        self.spi_touch.max_speed_hz = 1000000
        self.spi_touch.mode = 0
        
        # Pines GPIO
        self.cs_pin = 7   # T_CS
        self.irq_pin = 23 # T_IRQ
        
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self.cs_pin, GPIO.OUT)
        GPIO.setup(self.irq_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)
        
        # Calibración (ajustar según necesites)
        self.x_min = 200
        self.x_max = 3800
        self.y_min = 200
        self.y_max = 3800
        
    def clear_screen(self):
        """Limpiar pantalla"""
        self.draw.rectangle((0, 0, self.display.width, self.display.height), fill=(0, 0, 0))
        self.display.image(self.image)
        
    def draw_interface(self):
        """Dibujar interfaz inicial"""
        self.clear_screen()
        
        # Título
        try:
            font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16)
        except:
            font = ImageFont.load_default()
            
        self.draw.text((10, 10), "Touch Display Test", font=font, fill=(255, 255, 255))
        self.draw.text((10, 35), "Toca para dibujar", font=font, fill=(255, 255, 0))
        
        # Botones
        self.draw.rectangle((10, 200, 80, 230), fill=(255, 0, 0))
        self.draw.text((20, 210), "Clear", font=font, fill=(255, 255, 255))
        
        self.draw.rectangle((90, 200, 160, 230), fill=(0, 255, 0))
        self.draw.text((100, 210), "Green", font=font, fill=(0, 0, 0))
        
        self.draw.rectangle((170, 200, 240, 230), fill=(0, 0, 255))
        self.draw.text((180, 210), "Blue", font=font, fill=(255, 255, 255))
        
        self.display.image(self.image)
        
    def read_touch_raw(self):
        """Leer coordenadas raw del touch"""
        x_cmd = [0x90, 0x00, 0x00]
        y_cmd = [0xD0, 0x00, 0x00]
        
        GPIO.output(self.cs_pin, GPIO.LOW)
        
        x_data = self.spi_touch.xfer2(x_cmd)
        x_raw = ((x_data[1] << 8) | x_data[2]) >> 3
        
        y_data = self.spi_touch.xfer2(y_cmd)
        y_raw = ((y_data[1] << 8) | y_data[2]) >> 3
        
        GPIO.output(self.cs_pin, GPIO.HIGH)
        
        return x_raw, y_raw
    
    def is_touched(self):
        """Verificar si está siendo tocado"""
        return GPIO.input(self.irq_pin) == GPIO.LOW
    
    def get_position(self):
        """Obtener posición calibrada"""
        if not self.is_touched():
            return None
            
        x_raw, y_raw = self.read_touch_raw()
        
        # Calibrar
        x = int((x_raw - self.x_min) * self.display.width / (self.x_max - self.x_min))
        y = int((y_raw - self.y_min) * self.display.height / (self.y_max - self.y_min))
        
        # Limitar
        x = max(0, min(x, self.display.width - 1))
        y = max(0, min(y, self.display.height - 1))
        
        return (x, y)
    
    def handle_touch(self, pos):
        """Manejar eventos touch"""
        x, y = pos
        
        # Verificar botones
        if 200 <= y <= 230:
            if 10 <= x <= 80:  # Botón Clear
                self.draw_interface()
                return
            elif 90 <= x <= 160:  # Botón Green
                self.current_color = (0, 255, 0)
                return
            elif 170 <= x <= 240:  # Botón Blue
                self.current_color = (0, 0, 255)
                return
        
        # Dibujar en el área principal
        if y < 180:
            if self.last_pos:
                self.draw.line([self.last_pos, pos], fill=self.current_color, width=3)
            else:
                self.draw.ellipse([x-2, y-2, x+2, y+2], fill=self.current_color)
            
            self.display.image(self.image)
            self.last_pos = pos
    
    def run(self):
        """Ejecutar aplicación principal"""
        self.current_color = (255, 255, 255)  # Blanco por defecto
        self.draw_interface()
        
        print("Touch Display iniciado")
        print("Toca la pantalla para dibujar")
        print("Presiona Ctrl+C para salir")
        
        try:
            while True:
                if self.is_touched():
                    pos = self.get_position()
                    if pos:
                        self.handle_touch(pos)
                        if not self.drawing:
                            self.drawing = True
                            self.last_pos = pos
                else:
                    if self.drawing:
                        self.drawing = False
                        self.last_pos = None
                
                time.sleep(0.01)  # 100Hz
                
        except KeyboardInterrupt:
            print("\nSaliendo...")
            self.cleanup()
    
    def cleanup(self):
        """Limpiar recursos"""
        self.clear_screen()
        self.spi_touch.close()
        GPIO.cleanup()

# Programa principal
if __name__ == "__main__":
    app = TouchDisplay()
    app.run()