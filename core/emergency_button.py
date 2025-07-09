#!/usr/bin/env python3
import RPi.GPIO as GPIO
import time
import signal
import sys
import os

# Configuración del pin
EMERGENCY_BUTTON_PIN = 17

class EmergencyStop:
    def __init__(self):
        self.system_running = True
        self.setup_gpio()
        self.setup_signal_handlers()
        
    def setup_gpio(self):
        """Configura el GPIO para el botón de emergencia"""
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(EMERGENCY_BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
        print(f"🚨 Sistema de parada de emergencia activado - GPIO {EMERGENCY_BUTTON_PIN}")
        
    def setup_signal_handlers(self):
        """Configura manejadores de señales para cierre limpio"""
        signal.signal(signal.SIGINT, self.signal_handler)
        signal.signal(signal.SIGTERM, self.signal_handler)
        
    def emergency_stop_callback(self, channel):
        """Función que se ejecuta cuando se presiona el botón de emergencia"""
        print("\n" + "="*50)
        print("🚨 ¡PARADA DE EMERGENCIA ACTIVADA!")
        print("="*50)
        print(f"Botón presionado en GPIO {channel}")
        print("Deteniendo sistema...")
        
        # Aquí puedes agregar código para detener motores, procesos, etc.
        self.stop_all_processes()
        
        # Detener el sistema
        self.system_running = False
        
    def stop_all_processes(self):
        """Detiene todos los procesos del sistema"""
        print("🔴 Deteniendo motores...")
        # Ejemplo: detener motores, servos, etc.
        # motor.stop()
        # servo.stop()
        
        print("🔴 Cerrando conexiones...")
        # Ejemplo: cerrar conexiones, archivos, etc.
        # connection.close()
        
        print("🔴 Guardando estado...")
        # Ejemplo: guardar configuración, logs, etc.
        # save_configuration()
        
        print("✅ Sistema detenido de forma segura")
        
    def signal_handler(self, signum, frame):
        """Manejador para señales del sistema"""
        print(f"\n🔴 Señal recibida: {signum}")
        self.cleanup_and_exit()
        
    def cleanup_and_exit(self):
        """Limpia GPIO y sale del programa"""
        print("🧹 Limpiando GPIO...")
        GPIO.cleanup()
        print("👋 Programa terminado")
        sys.exit(0)
        
    def test_emergency_button(self):
        """Test del botón de emergencia con interrupciones"""
        print("\n=== TEST BOTÓN DE EMERGENCIA ===")
        print("Estado inicial del botón:")
        
        # Verificar estado inicial
        initial_state = GPIO.input(EMERGENCY_BUTTON_PIN)
        if initial_state == GPIO.HIGH:
            print("✅ Botón en estado normal (HIGH)")
        else:
            print("⚠️  Botón presionado o problema con pull-up (LOW)")
            
        # Configurar interrupción para flanco descendente (botón presionado)
        GPIO.add_event_detect(
            EMERGENCY_BUTTON_PIN, 
            GPIO.FALLING, 
            callback=self.emergency_stop_callback, 
            bouncetime=300  # 300ms para evitar rebotes
        )
        
        print("\n🚨 Sistema de emergencia activo")
        print("💡 Presiona el botón para activar parada de emergencia")
        print("💡 Presiona Ctrl+C para salir normalmente")
        print("-" * 50)
        
        try:
            counter = 0
            while self.system_running:
                # Simular trabajo del sistema
                counter += 1
                print(f"🔄 Sistema funcionando... [{counter}]", end='\r')
                time.sleep(1)
                
        except KeyboardInterrupt:
            print("\n\n💡 Salida normal solicitada")
            
        finally:
            self.cleanup_and_exit()
            
    def test_button_status(self):
        """Test simple para verificar el estado del botón"""
        print("\n=== VERIFICACIÓN DEL BOTÓN ===")
        
        for i in range(10):
            state = GPIO.input(EMERGENCY_BUTTON_PIN)
            status = "🟢 NORMAL" if state == GPIO.HIGH else "🔴 PRESIONADO"
            print(f"Estado {i+1}/10: {status}")
            time.sleep(0.5)
            
        print("\n✅ Verificación completada")

def main():
    """Función principal"""
    print("🚨 SISTEMA DE PARADA DE EMERGENCIA")
    print("="*40)
    print("Pin: GPIO 17")
    print("Configuración: Pull-up interno")
    print("Conexión: GPIO 17 → Botón → GND")
    print("="*40)
    
    emergency_system = EmergencyStop()
    
    try:
        while True:
            print("\nSelecciona una opción:")
            print("1. Test de estado del botón")
            print("2. Activar sistema de emergencia")
            print("3. Salir")
            
            choice = input("\nIngresa tu opción (1-3): ").strip()
            
            if choice == '1':
                emergency_system.test_button_status()
            elif choice == '2':
                emergency_system.test_emergency_button()
            elif choice == '3':
                print("Saliendo...")
                break
            else:
                print("❌ Opción inválida")
                
    except KeyboardInterrupt:
        print("\n\nPrograma interrumpido")
    finally:
        emergency_system.cleanup_and_exit()

if __name__ == "__main__":
    main()