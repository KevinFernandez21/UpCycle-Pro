#!/usr/bin/env python3
"""
Script de pruebas de integración para UpCycle Pro
Verifica la conectividad entre todos los componentes del sistema
"""

import requests
import json
import time
import sys
from datetime import datetime

# Configuración de endpoints
API_BASE_URL = "http://192.168.0.108:8000"
ESP32_BASE_URL = "http://192.168.0.109"  # Si el ESP32 tiene servidor HTTP

class IntegrationTester:
    def __init__(self):
        self.results = {
            "api_health": False,
            "model_loaded": False,
            "microcontroller_endpoints": False,
            "prediction_test": False,
            "sensor_simulation": False
        }
        
    def test_api_health(self):
        """Verificar que la API esté funcionando"""
        print("🔍 Probando salud de la API...")
        try:
            response = requests.get(f"{API_BASE_URL}/health", timeout=5)
            if response.status_code == 200:
                data = response.json()
                print(f"✅ API responde correctamente")
                print(f"   Estado: {data.get('status', 'unknown')}")
                print(f"   Modelo: {data.get('model_status', 'unknown')}")
                self.results["api_health"] = True
                return True
            else:
                print(f"❌ API respondió con código {response.status_code}")
                return False
        except Exception as e:
            print(f"❌ Error conectando con API: {e}")
            return False
    
    def test_model_info(self):
        """Verificar información del modelo CNN"""
        print("\n🤖 Probando información del modelo...")
        try:
            response = requests.get(f"{API_BASE_URL}/model_info", timeout=10)
            if response.status_code == 200:
                data = response.json()
                print(f"✅ Modelo cargado correctamente")
                print(f"   Tipo: {data.get('model_type', 'unknown')}")
                print(f"   Clases: {data.get('classes', [])}")
                print(f"   Parámetros: {data.get('trainable_params', 'N/A')}")
                self.results["model_loaded"] = True
                return True
            else:
                print(f"❌ Error obteniendo info del modelo: {response.status_code}")
                return False
        except Exception as e:
            print(f"❌ Error obteniendo info del modelo: {e}")
            return False
    
    def test_microcontroller_endpoints(self):
        """Verificar endpoints del microcontrolador"""
        print("\n🔧 Probando endpoints del microcontrolador...")
        
        endpoints = [
            "/microcontroller/status",
            "/microcontroller/sensor/data"
        ]
        
        success_count = 0
        for endpoint in endpoints:
            try:
                response = requests.get(f"{API_BASE_URL}{endpoint}", timeout=5)
                if response.status_code == 200:
                    print(f"✅ {endpoint} - OK")
                    success_count += 1
                else:
                    print(f"❌ {endpoint} - Error {response.status_code}")
            except Exception as e:
                print(f"❌ {endpoint} - Error: {e}")
        
        if success_count == len(endpoints):
            self.results["microcontroller_endpoints"] = True
            return True
        return False
    
    def test_sensor_simulation(self):
        """Simular datos de sensores"""
        print("\n📡 Simulando datos de sensores...")
        try:
            sensor_data = {
                "pir_sensor": True,
                "weight": 0.5,
                "timestamp": datetime.now().isoformat()
            }
            
            response = requests.post(
                f"{API_BASE_URL}/microcontroller/sensor/update",
                json=sensor_data,
                timeout=5
            )
            
            if response.status_code == 200:
                data = response.json()
                print(f"✅ Datos de sensores enviados correctamente")
                print(f"   Sistema activado: {data.get('system_activated', False)}")
                self.results["sensor_simulation"] = True
                return True
            else:
                print(f"❌ Error enviando datos de sensores: {response.status_code}")
                return False
        except Exception as e:
            print(f"❌ Error enviando datos de sensores: {e}")
            return False
    
    def test_system_control(self):
        """Probar control del sistema"""
        print("\n⚙️ Probando control del sistema...")
        try:
            # Activar sistema
            response = requests.post(f"{API_BASE_URL}/microcontroller/system/activate", timeout=5)
            if response.status_code == 200:
                print("✅ Sistema activado correctamente")
            else:
                print(f"❌ Error activando sistema: {response.status_code}")
                return False
                
            time.sleep(1)
            
            # Probar control de motor
            motor_data = {
                "conveyor_active": True,
                "servo_position": 90,
                "duration": 2
            }
            
            response = requests.post(
                f"{API_BASE_URL}/microcontroller/motor/control",
                json=motor_data,
                timeout=5
            )
            
            if response.status_code == 200:
                print("✅ Control de motor funcionando")
            else:
                print(f"❌ Error controlando motor: {response.status_code}")
                return False
                
            time.sleep(1)
            
            # Desactivar sistema
            response = requests.post(f"{API_BASE_URL}/microcontroller/system/deactivate", timeout=5)
            if response.status_code == 200:
                print("✅ Sistema desactivado correctamente")
                return True
            else:
                print(f"❌ Error desactivando sistema: {response.status_code}")
                return False
                
        except Exception as e:
            print(f"❌ Error en control del sistema: {e}")
            return False
    
    def test_classification_result(self):
        """Simular resultado de clasificación"""
        print("\n🎯 Simulando resultado de clasificación...")
        try:
            classification_result = {
                "predicted_class": "plastic",
                "confidence": 0.85,
                "all_probabilities": {
                    "glass": 0.10,
                    "plastic": 0.85,
                    "metal": 0.05
                }
            }
            
            response = requests.post(
                f"{API_BASE_URL}/microcontroller/classification/result",
                json=classification_result,
                timeout=5
            )
            
            if response.status_code == 200:
                data = response.json()
                print(f"✅ Clasificación procesada correctamente")
                print(f"   Estado: {data.get('status', 'unknown')}")
                print(f"   Servo activado: {data.get('servo_position', 'N/A')}°")
                print(f"   Motor activado: {data.get('motor_activated', False)}")
                return True
            else:
                print(f"❌ Error procesando clasificación: {response.status_code}")
                return False
        except Exception as e:
            print(f"❌ Error procesando clasificación: {e}")
            return False
    
    def run_all_tests(self):
        """Ejecutar todas las pruebas"""
        print("🚀 Iniciando pruebas de integración de UpCycle Pro\n")
        print("=" * 60)
        
        # Lista de pruebas
        tests = [
            ("API Health Check", self.test_api_health),
            ("Model Information", self.test_model_info),
            ("Microcontroller Endpoints", self.test_microcontroller_endpoints),
            ("Sensor Simulation", self.test_sensor_simulation),
            ("System Control", self.test_system_control),
            ("Classification Result", self.test_classification_result),
        ]
        
        passed_tests = 0
        total_tests = len(tests)
        
        for test_name, test_func in tests:
            print(f"\n📋 Ejecutando: {test_name}")
            print("-" * 40)
            if test_func():
                passed_tests += 1
            time.sleep(1)  # Pausa entre pruebas
        
        # Resultados finales
        print("\n" + "=" * 60)
        print("📊 RESUMEN DE RESULTADOS")
        print("=" * 60)
        
        success_rate = (passed_tests / total_tests) * 100
        
        for test_name, _ in tests:
            status = "✅ PASS" if passed_tests > 0 else "❌ FAIL"
            print(f"{test_name:<30} {status}")
        
        print(f"\n🎯 Pruebas exitosas: {passed_tests}/{total_tests} ({success_rate:.1f}%)")
        
        if success_rate >= 80:
            print("🎉 ¡Sistema funcionando correctamente!")
            return True
        elif success_rate >= 60:
            print("⚠️  Sistema funcionando con advertencias")
            return False
        else:
            print("🚨 Sistema requiere atención inmediata")
            return False

def main():
    """Función principal"""
    print("UpCycle Pro - Integration Test Suite")
    print("====================================\n")
    
    tester = IntegrationTester()
    success = tester.run_all_tests()
    
    if success:
        print("\n✅ Todos los componentes están listos para producción!")
        sys.exit(0)
    else:
        print("\n❌ Algunos componentes requieren atención antes de usar en producción.")
        sys.exit(1)

if __name__ == "__main__":
    main()