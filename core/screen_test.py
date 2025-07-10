import xpt2046_circuitpython
import time
import busio
import digitalio
from board import SCK, MOSI, MISO, D24, D22

# Pin config
T_CS_PIN = D24
T_IRQ_PIN = D22

# Set up SPI bus using hardware SPI
spi = busio.SPI(clock=SCK, MOSI=MOSI, MISO=MISO)
# Create touch controller
touch = xpt2046.Touch(
    spi, 
    cs = digitalio.DigitalInOut(T_CS_PIN),
    interrupt = digitalio.DigitalInOut(T_IRQ_PIN)
)

# Check if we have an interrupt signal
if touch.is_pressed():
    # Get the coordinates for this touch
    print(touch.get_coordinates())