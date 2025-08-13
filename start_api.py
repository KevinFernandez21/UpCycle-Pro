#!/usr/bin/env python3
"""
Script para iniciar el servidor FastAPI con los modelos CNN
"""
import os
import sys

# Agregar el directorio de la API al path
api_dir = os.path.join(os.path.dirname(__file__), 'backend', 'api')
sys.path.insert(0, api_dir)

if __name__ == "__main__":
    # Cambiar al directorio de la API
    os.chdir(api_dir)
    
    try:
        import uvicorn
        print("🚀 Starting FastAPI server with CNN models...")
        print(f"📂 Working directory: {os.getcwd()}")
        print(f"🤖 CNN models folder: ../ai_client/CNN")
        print(f"🌐 API will be available at: http://localhost:8000")
        print(f"📚 Documentation at: http://localhost:8000/docs")
        
        # Iniciar el servidor
        uvicorn.run(
            "main:app",
            host="localhost",  # Cambiar a localhost para pruebas locales
            port=8000,
            reload=True,
            log_level="info"
        )
    except ImportError as e:
        print(f"❌ Error: {e}")
        print("Please install the requirements first:")
        print("cd backend/api && pip install -r requirements.txt")
    except Exception as e:
        print(f"❌ Error starting server: {e}")