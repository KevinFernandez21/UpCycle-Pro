#!/usr/bin/env python3
import requests
import json

# Test básico de la API
def test_api():
    base_url = "http://localhost:8000"
    
    print("🧪 Testing FastAPI CNN Model API...")
    
    # Test 1: Health check
    try:
        response = requests.get(f"{base_url}/health")
        print(f"✅ Health check: {response.status_code} - {response.json()}")
    except Exception as e:
        print(f"❌ Health check failed: {e}")
        return
    
    # Test 2: Available models
    try:
        response = requests.get(f"{base_url}/available_models")
        print(f"✅ Available models: {response.status_code}")
        models_data = response.json()
        print(f"   Found {models_data.get('count', 0)} models:")
        for model in models_data.get('models', []):
            print(f"   - {model['filename']} ({model['size_mb']} MB)")
    except Exception as e:
        print(f"❌ Available models failed: {e}")
    
    # Test 3: Load model
    try:
        response = requests.post(f"{base_url}/load_model")
        print(f"✅ Load model: {response.status_code}")
        if response.status_code == 200:
            print(f"   Model loaded successfully: {response.json()['message']}")
    except Exception as e:
        print(f"❌ Load model failed: {e}")
    
    # Test 4: Model info
    try:
        response = requests.get(f"{base_url}/model_info")
        print(f"✅ Model info: {response.status_code}")
        if response.status_code == 200:
            model_info = response.json()
            print(f"   Model type: {model_info.get('model_type', 'N/A')}")
            print(f"   Input shape: {model_info.get('input_shape', 'N/A')}")
            print(f"   Classes: {model_info.get('classes', 'N/A')}")
    except Exception as e:
        print(f"❌ Model info failed: {e}")

if __name__ == "__main__":
    test_api()