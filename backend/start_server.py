#!/usr/bin/env python3
"""
UpCycle Pro Backend Server Startup Script
=========================================

This script starts the FastAPI server with optimal configuration for the UpCycle Pro system.
It handles both development and production environments.

Usage:
    python start_server.py [--dev|--prod] [--host HOST] [--port PORT]

Examples:
    python start_server.py --dev                    # Development mode
    python start_server.py --prod --port 8080       # Production mode on port 8080
    python start_server.py --host 192.168.1.100     # Specific host
"""

import argparse
import asyncio
import logging
import os
import sys
import uvicorn
from pathlib import Path

# Add the API directory to the Python path
api_dir = Path(__file__).parent / "api"
sys.path.insert(0, str(api_dir))

def setup_logging(level: str = "INFO"):
    """Configure logging for the application"""
    logging.basicConfig(
        level=getattr(logging, level.upper()),
        format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S"
    )
    
    # Reduce noise from some libraries
    logging.getLogger("uvicorn.access").setLevel(logging.WARNING)
    logging.getLogger("aiohttp.access").setLevel(logging.WARNING)

def check_dependencies():
    """Check if all required dependencies are installed"""
    required_packages = [
        "fastapi",
        "uvicorn",
        "aiohttp",
        "tensorflow",
        "numpy",
        "pillow",
        "pydantic",
    ]
    
    missing_packages = []
    for package in required_packages:
        try:
            __import__(package)
        except ImportError:
            missing_packages.append(package)
    
    if missing_packages:
        print("❌ Missing required packages:")
        for package in missing_packages:
            print(f"   - {package}")
        print("\nPlease install dependencies:")
        print("   pip install -r requirements.txt")
        sys.exit(1)
    
    print("✅ All dependencies are installed")

def check_model_files():
    """Check if CNN model files are available"""
    model_dir = Path("../ai_client/CNN")
    if not model_dir.exists():
        print(f"⚠️ Model directory not found: {model_dir}")
        print("   The server will start with a dummy model for testing")
        return False
    
    model_extensions = [".keras", ".h5"]
    model_files = []
    
    for ext in model_extensions:
        model_files.extend(model_dir.glob(f"*{ext}"))
    
    if model_files:
        print(f"✅ Found {len(model_files)} model file(s):")
        for model_file in model_files[:3]:  # Show first 3
            print(f"   - {model_file.name}")
        if len(model_files) > 3:
            print(f"   ... and {len(model_files) - 3} more")
        return True
    else:
        print("⚠️ No CNN model files found")
        print("   The server will start with a dummy model for testing")
        return False

def print_banner():
    """Print startup banner"""
    banner = """
    ╔══════════════════════════════════════════════════════════════╗
    ║                        UpCycle Pro                           ║
    ║                    Backend API Server                        ║
    ║                                                              ║
    ║        🤖 AI-Powered Material Classification System         ║
    ║        🔗 ESP32 Integration & Real-time WebSockets          ║
    ║        📊 MQTT & Node-RED Compatible                        ║
    ╚══════════════════════════════════════════════════════════════╝
    """
    print(banner)

def print_server_info(host: str, port: int, dev_mode: bool):
    """Print server information"""
    mode = "Development" if dev_mode else "Production"
    print(f"🚀 Starting {mode} Server")
    print(f"📡 Host: {host}")
    print(f"🔌 Port: {port}")
    print(f"📚 API Documentation: http://{host}:{port}/docs")
    print(f"🔄 WebSocket Endpoint: ws://{host}:{port}/ws/flutter_app")
    print(f"📸 Camera Stream: ws://{host}:{port}/ws/camera_stream")
    print()

async def test_system_components():
    """Test system components before starting server"""
    print("🧪 Testing system components...")
    
    # Test model loading
    try:
        import tensorflow as tf
        print("✅ TensorFlow is working")
    except Exception as e:
        print(f"❌ TensorFlow error: {e}")
    
    # Test image processing
    try:
        from PIL import Image
        import numpy as np
        test_img = Image.new('RGB', (224, 224), color='red')
        img_array = np.array(test_img)
        print("✅ Image processing is working")
    except Exception as e:
        print(f"❌ Image processing error: {e}")
    
    # Test async HTTP client
    try:
        import aiohttp
        print("✅ Async HTTP client is working")
    except Exception as e:
        print(f"❌ HTTP client error: {e}")
    
    print()

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="Start the UpCycle Pro Backend Server",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    
    parser.add_argument(
        "--dev", 
        action="store_true", 
        help="Run in development mode with auto-reload"
    )
    parser.add_argument(
        "--prod", 
        action="store_true", 
        help="Run in production mode"
    )
    parser.add_argument(
        "--host", 
        default="0.0.0.0", 
        help="Host to bind to (default: 0.0.0.0)"
    )
    parser.add_argument(
        "--port", 
        type=int, 
        default=8000, 
        help="Port to bind to (default: 8000)"
    )
    parser.add_argument(
        "--log-level", 
        default="INFO", 
        choices=["DEBUG", "INFO", "WARNING", "ERROR"],
        help="Logging level (default: INFO)"
    )
    parser.add_argument(
        "--workers", 
        type=int, 
        default=1,
        help="Number of worker processes (production only, default: 1)"
    )
    parser.add_argument(
        "--no-tests", 
        action="store_true",
        help="Skip system component tests"
    )
    
    args = parser.parse_args()
    
    # Determine mode
    if args.prod:
        dev_mode = False
        reload = False
    else:
        dev_mode = True
        reload = True
    
    # Setup
    print_banner()
    setup_logging(args.log_level)
    
    if not args.no_tests:
        print("🔍 Checking system requirements...")
        check_dependencies()
        check_model_files()
        
        # Run async tests
        asyncio.run(test_system_components())
    
    print_server_info(args.host, args.port, dev_mode)
    
    # Configure uvicorn
    config = {
        "app": "main:app",
        "host": args.host,
        "port": args.port,
        "reload": reload,
        "log_level": args.log_level.lower(),
        "access_log": dev_mode,
        "use_colors": True,
    }
    
    if not dev_mode:
        config["workers"] = args.workers
    
    try:
        print("🎯 Starting server...")
        uvicorn.run(**config)
    except KeyboardInterrupt:
        print("\n👋 Server stopped by user")
    except Exception as e:
        print(f"\n❌ Server error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()