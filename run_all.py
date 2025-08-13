#!/usr/bin/env python3
"""
Script para ejecutar tanto la API como el Dashboard
"""
import subprocess
import sys
import os
import time
import threading

def run_api():
    """Ejecutar FastAPI"""
    try:
        print("🚀 Iniciando FastAPI...")
        os.chdir('backend/api')
        subprocess.run([sys.executable, "../../start_api.py"])
    except Exception as e:
        print(f"❌ Error ejecutando API: {e}")

def run_dashboard():
    """Ejecutar Dashboard de Streamlit"""
    try:
        # Esperar un poco para que la API se inicie
        time.sleep(3)
        print("📊 Iniciando Dashboard...")
        subprocess.run([
            sys.executable, "-m", "streamlit", "run", "dashboard.py",
            "--server.port=8501",
            "--server.address=localhost",
            "--browser.gatherUsageStats=false"
        ])
    except Exception as e:
        print(f"❌ Error ejecutando Dashboard: {e}")

def main():
    print("🔧 UpCycle Pro - Sistema Completo")
    print("=" * 40)
    print("🤖 FastAPI: http://localhost:8000")
    print("📊 Dashboard: http://localhost:8501")
    print("📚 Docs: http://localhost:8000/docs")
    print("=" * 40)
    
    try:
        # Crear threads para ejecutar ambos servicios
        api_thread = threading.Thread(target=run_api, daemon=True)
        dashboard_thread = threading.Thread(target=run_dashboard, daemon=True)
        
        # Iniciar API primero
        api_thread.start()
        
        # Esperar un poco y luego iniciar dashboard
        time.sleep(2)
        dashboard_thread.start()
        
        # Mantener el programa corriendo
        api_thread.join()
        dashboard_thread.join()
        
    except KeyboardInterrupt:
        print("\n🛑 Servicios detenidos")
    except Exception as e:
        print(f"❌ Error: {e}")

if __name__ == "__main__":
    main()