import time
import board
import busio
from adafruit_pca9685 import PCA9685
from adafruit_motor import servo

def inicializar_pca9685(reintentos=5):
    """Inicializa PCA9685 con reintentos"""
    for intento in range(reintentos):
        try:
            print(f"Intento {intento + 1} de conectar con PCA9685...")
            i2c = busio.I2C(board.SCL, board.SDA)
            pca = PCA9685(i2c)
            pca.frequency = 50
            print("✓ PCA9685 conectado exitosamente")
            return pca
        except Exception as e:
            print(f"✗ Error en intento {intento + 1}: {e}")
            if intento < reintentos - 1:
                print("Esperando 2 segundos antes del siguiente intento...")
                time.sleep(2)
            else:
                print("No se pudo conectar con PCA9685")
                return None

def main():
    # Inicializar PCA9685
    pca = inicializar_pca9685()
    if pca is None:
        print("Error: No se pudo inicializar PCA9685")
        return
    
    try:
        # Crear objeto servo en el canal 0
        servo_motor = servo.Servo(pca.channels[0])
        print("✓ Servo configurado en canal 0")
        
        print("Iniciando movimiento del servo...")
        print("Presiona Ctrl+C para salir")
        
        while True:
            # Mover servo a 0 grados
            servo_motor.angle = 0
            print("Servo a 0 grados")
            time.sleep(1)
            
            # Mover servo a 90 grados
            servo_motor.angle = 90
            print("Servo a 90 grados")
            time.sleep(1)
            
            # Mover servo a 180 grados
            servo_motor.angle = 180
            print("Servo a 180 grados")
            time.sleep(1)
            
    except KeyboardInterrupt:
        print("\nPrograma interrumpido por el usuario")
        
    except Exception as e:
        print(f"Error durante la ejecución: {e}")
        
    finally:
        # Limpiar recursos
        if 'pca' in locals():
            pca.deinit()
        print("Recursos liberados")

if __name__ == "__main__":
    main()