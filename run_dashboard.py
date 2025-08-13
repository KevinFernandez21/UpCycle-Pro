#!/usr/bin/env python3
"""
Script para ejecutar el dashboard de Streamlit
"""
import subprocess
import sys
import os

def main():
    try:
        print("🚀 Iniciando Dashboard de UpCycle Pro...")
        print("📱 El dashboard se abrirá en tu navegador")
        print("🔗 URL: http://localhost:8501")
        print("⏹️  Presiona Ctrl+C para detener")
        print()
        
        # Ejecutar Streamlit
        subprocess.run([
            sys.executable, "-m", "streamlit", "run", "dashboard.py",
            "--server.port=8501",
            "--server.address=localhost",
            "--browser.gatherUsageStats=false"
        ])
        
    except KeyboardInterrupt:
        print("\n🛑 Dashboard detenido")
    except FileNotFoundError:
        print("❌ Error: Streamlit no está instalado")
        print("💡 Instala las dependencias: pip install -r dashboard_requirements.txt")
    except Exception as e:
        print(f"❌ Error ejecutando el dashboard: {e}")

if __name__ == "__main__":
    main()