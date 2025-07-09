from picamera2 import Picamera2,Prewiew
from time import sleep  

picam2 = PiCamera2()
picam2.start_preview(Preview.QTGL)
sleep(60)
picam2.close()  # Espera para que la cámara se inicialice