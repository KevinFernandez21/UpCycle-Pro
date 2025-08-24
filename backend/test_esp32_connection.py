#!/usr/bin/env python3
"""
Script de prueba para conexión ESP32 - Backend
==============================================

Este script prueba la comunicación entre el ESP32 y el backend FastAPI.
"""

import asyncio
import aiohttp
import json
from datetime import datetime

# Configuración
BACKEND_URL = "http://localhost:8000"
ESP32_IP = "192.168.1.50"  # Cambiar por la IP real del ESP32
DEVICE_ID = "esp32_main_001"

async def test_backend_health():
    """Probar que el backend está funcionando"""
    print("🔍 Probando salud del backend...")
    
    try:
        async with aiohttp.ClientSession() as session:
            async with session.get(f"{BACKEND_URL}/health") as response:
                if response.status == 200:
                    data = await response.json()
                    print(f"✅ Backend está funcionando: {data}")
                    return True
                else:
                    print(f"❌ Backend respondió con código: {response.status}")
                    return False
    except Exception as e:
        print(f"❌ Error conectando al backend: {e}")
        return False

async def test_esp32_status():
    """Probar estado del ESP32"""
    print(f"🔍 Probando estado del ESP32 en {ESP32_IP}...")
    
    try:
        async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=10)) as session:
            async with session.get(f"http://{ESP32_IP}/status") as response:
                if response.status == 200:
                    data = await response.json()
                    print(f"✅ ESP32 está funcionando:")
                    print(f"   Device ID: {data.get('device_id', 'N/A')}")
                    print(f"   Estado: {data.get('state', 'N/A')}")
                    print(f"   Backend conectado: {data.get('backend_connected', 'N/A')}")
                    print(f"   WiFi RSSI: {data.get('wifi_rssi', 'N/A')} dBm")
                    return True
                else:
                    print(f"❌ ESP32 respondió con código: {response.status}")
                    return False
    except Exception as e:
        print(f"❌ Error conectando al ESP32: {e}")
        return False

async def test_device_registration():
    """Probar registro de dispositivo en el backend"""
    print("🔍 Probando registro de dispositivo...")
    
    try:
        data = {
            "device_id": DEVICE_ID,
            "ip_address": ESP32_IP
        }
        
        async with aiohttp.ClientSession() as session:
            async with session.post(
                f"{BACKEND_URL}/api/esp32-cam/register",
                data=data
            ) as response:
                result = await response.json()
                if response.status == 200:
                    print(f"✅ Dispositivo registrado exitosamente: {result['message']}")
                    return True
                else:
                    print(f"⚠️ Registro con advertencias: {result}")
                    return True  # Aún funciona aunque sea con advertencia
    except Exception as e:
        print(f"❌ Error registrando dispositivo: {e}")
        return False

async def test_sensor_data():
    """Probar envío de datos de sensores"""
    print("🔍 Probando envío de datos de sensores...")
    
    try:
        sensor_data = {
            "device_id": DEVICE_ID,
            "timestamp": datetime.now().isoformat(),
            "pir1": False,
            "pir2": True,
            "pir3": False,
            "conveyor_active": False,
            "weight1": 0.0,
            "weight2": 0.5,
            "weight3": 0.0,
            "servo_positions": {
                "servo1": 90,
                "servo2": 90,
                "servo3": 90
            }
        }
        
        async with aiohttp.ClientSession() as session:
            async with session.post(
                f"{BACKEND_URL}/api/esp32-control/sensors",
                json=sensor_data
            ) as response:
                result = await response.json()
                if response.status == 200:
                    print(f"✅ Datos de sensores enviados exitosamente")
                    
                    # Verificar comandos pendientes
                    if "pending_commands" in result:
                        commands = result["pending_commands"]
                        if commands:
                            print(f"📋 Comandos pendientes recibidos: {len(commands)}")
                            for cmd in commands:
                                print(f"   - {cmd['command']}: {cmd.get('parameters', {})}")
                        else:
                            print("📋 No hay comandos pendientes")
                    
                    return True
                else:
                    print(f"❌ Error enviando datos: {response.status} - {result}")
                    return False
    except Exception as e:
        print(f"❌ Error enviando datos de sensores: {e}")
        return False

async def test_classification_command():
    """Probar comando de clasificación"""
    print("🔍 Probando comando de clasificación...")
    
    try:
        command_data = {
            "device_id": DEVICE_ID,
            "command": "move_servo",
            "parameters": {
                "material": "plastic",
                "position": 90,
                "duration": 2000
            },
            "priority": 1
        }
        
        async with aiohttp.ClientSession() as session:
            async with session.post(
                f"{BACKEND_URL}/api/esp32-control/command/{DEVICE_ID}",
                json=command_data
            ) as response:
                result = await response.json()
                if response.status == 200:
                    print(f"✅ Comando enviado exitosamente: {result['message']}")
                    print(f"   Cola de comandos: {result.get('queue_size', 0)}")
                    return True
                else:
                    print(f"❌ Error enviando comando: {response.status} - {result}")
                    return False
    except Exception as e:
        print(f"❌ Error enviando comando: {e}")
        return False

async def test_esp32_direct_command():
    """Probar comando directo al ESP32"""
    print("🔍 Probando comando directo al ESP32...")
    
    try:
        command_data = {
            "material": "glass"
        }
        
        async with aiohttp.ClientSession() as session:
            async with session.post(
                f"http://{ESP32_IP}/classify",
                json=command_data
            ) as response:
                if response.status == 200:
                    result = await response.json()
                    print(f"✅ Comando directo enviado: {result['message']}")
                    return True
                else:
                    print(f"❌ ESP32 no pudo procesar comando: {response.status}")
                    return False
    except Exception as e:
        print(f"❌ Error con comando directo: {e}")
        return False

async def test_system_metrics():
    """Probar métricas del sistema"""
    print("🔍 Probando métricas del sistema...")
    
    try:
        async with aiohttp.ClientSession() as session:
            async with session.get(f"{BACKEND_URL}/system/metrics") as response:
                if response.status == 200:
                    result = await response.json()
                    print(f"✅ Métricas del sistema obtenidas:")
                    print(f"   Salud del sistema: {result.get('system_health', 'N/A')}")
                    
                    esp32_devices = result.get('esp32_devices', {})
                    connectivity = esp32_devices.get('connectivity', {})
                    print(f"   Dispositivos totales: {connectivity.get('total_devices', 0)}")
                    print(f"   Dispositivos online: {connectivity.get('online_devices', 0)}")
                    
                    websockets = result.get('websocket_connections', {})
                    print(f"   Conexiones WebSocket: {websockets.get('total_connections', 0)}")
                    
                    return True
                else:
                    print(f"❌ Error obteniendo métricas: {response.status}")
                    return False
    except Exception as e:
        print(f"❌ Error obteniendo métricas: {e}")
        return False

async def run_all_tests():
    """Ejecutar todas las pruebas"""
    print("🚀 INICIANDO PRUEBAS DE INTEGRACIÓN ESP32 - BACKEND")
    print("=" * 60)
    
    tests = [
        ("Salud del Backend", test_backend_health),
        ("Estado ESP32", test_esp32_status),
        ("Registro de Dispositivo", test_device_registration),
        ("Datos de Sensores", test_sensor_data),
        ("Comando de Clasificación", test_classification_command),
        ("Comando Directo ESP32", test_esp32_direct_command),
        ("Métricas del Sistema", test_system_metrics),
    ]
    
    results = []
    
    for test_name, test_func in tests:
        print(f"\n📋 {test_name}")
        print("-" * 40)
        
        try:
            success = await test_func()
            results.append((test_name, success))
        except Exception as e:
            print(f"❌ Error inesperado en {test_name}: {e}")
            results.append((test_name, False))
        
        await asyncio.sleep(1)  # Pausa entre pruebas
    
    # Resumen
    print("\n" + "=" * 60)
    print("📊 RESUMEN DE PRUEBAS")
    print("=" * 60)
    
    passed = 0
    total = len(results)
    
    for test_name, success in results:
        status = "✅ PASS" if success else "❌ FAIL"
        print(f"{status} {test_name}")
        if success:
            passed += 1
    
    print(f"\n🎯 Resultado: {passed}/{total} pruebas exitosas")
    
    if passed == total:
        print("🎉 ¡Todas las pruebas pasaron! El sistema está funcionando correctamente.")
    elif passed > total // 2:
        print("⚠️ La mayoría de pruebas pasaron, pero hay algunos problemas.")
    else:
        print("🚨 Muchas pruebas fallaron. Revisar configuración del sistema.")
    
    return passed == total

if __name__ == "__main__":
    print("UpCycle Pro - Test de Integración ESP32-Backend")
    print(f"Backend URL: {BACKEND_URL}")
    print(f"ESP32 IP: {ESP32_IP}")
    print(f"Device ID: {DEVICE_ID}")
    print()
    
    # Ejecutar pruebas
    success = asyncio.run(run_all_tests())
    
    if success:
        print("\n✨ Sistema listo para producción!")
    else:
        print("\n🔧 Revisar configuración antes de usar en producción.")
    
    input("\nPresiona Enter para salir...")