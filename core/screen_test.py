import board
import displayio
import adafruit_ili9341
import adafruit_xpt2046
import digitalio
import time

# Configurar SPI
spi = board.SPI()

# Pines del display
tft_cs = board.CE0
tft_dc = board.D24
tft_reset = board.D25

# Pines del touch
touch_cs = board.CE1
touch_irq = board.D23

# Inicializar display
displayio.release_displays()
display_bus = displayio.FourWire(spi, command=tft_dc, chip_select=tft_cs, reset=tft_reset)
display = adafruit_ili9341.ILI9341(display_bus, width=240, height=320)

# Inicializar touch
touch = adafruit_xpt2046.XPT2046(spi, cs=touch_cs, irq=touch_irq)

# Crear grupo principal
splash = displayio.Group()
display.show(splash)

print("Pantalla ILI9341 con touch conectada")

# Loop para detectar toques
while True:
    if touch.touched:
        point = touch.touch_point
        if point:
            print(f"Touch detectado en: x={point[0]}, y={point[1]}")
    time.sleep(0.1)