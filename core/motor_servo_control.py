import time
import board
import busio
from adafruit_pca9685 import PCA9685
from adafruit_motor import servo

# Inicializar I2C y PCA9685
i2c = busio.I2C(board.SCL, board.SDA)
pca = PCA9685(i2c)
pca.frequency = 50  # 50Hz para servos

# Crear objeto servo en el canal 0
servo_motor = servo.Servo(pca.channels[0])

# Mover el servo
while True:
    servo_motor.angle = 0    # 0 grados
    time.sleep(1)
    servo_motor.angle = 90   # 90 grados
    time.sleep(1)
    servo_motor.angle = 180  # 180 grados
    time.sleep(1)