import os
from typing import List
from pydantic_settings import BaseSettings
from pydantic import Field

class Settings(BaseSettings):
    # API Configuration
    API_HOST: str = Field(default="0.0.0.0", description="API host")
    API_PORT: int = Field(default=8000, description="API port")
    API_RELOAD: bool = Field(default=True, description="Enable auto-reload in development")
    
    # CORS Configuration
    CORS_ORIGINS: List[str] = Field(
        default=["*"], 
        description="Allowed CORS origins"
    )
    
    # ESP32 Configuration
    ESP32_DISCOVERY_ENABLED: bool = Field(default=True, description="Enable ESP32 auto-discovery")
    ESP32_DISCOVERY_INTERVAL: int = Field(default=300, description="Discovery interval in seconds")
    ESP32_TIMEOUT: int = Field(default=10, description="ESP32 communication timeout")
    ESP32_RETRY_ATTEMPTS: int = Field(default=3, description="ESP32 retry attempts")
    
    # Network Configuration
    NETWORK_BASE_IP: str = Field(default="192.168.1.", description="Base IP for device discovery")
    NETWORK_SCAN_RANGE: int = Field(default=254, description="IP range to scan")
    
    # CNN Model Configuration
    MODEL_PATH: str = Field(default="../ai_client/CNN", description="Path to CNN models")
    MODEL_CLASSES: List[str] = Field(
        default=["glass", "metal", "plastic"], 
        description="Classification classes"
    )
    MODEL_INPUT_SIZE: tuple = Field(default=(224, 224), description="Model input image size")
    MODEL_CONFIDENCE_THRESHOLD: float = Field(default=0.7, description="Minimum confidence for action")
    
    # WebSocket Configuration
    WS_MAX_CONNECTIONS: int = Field(default=100, description="Maximum WebSocket connections")
    WS_HEARTBEAT_INTERVAL: int = Field(default=30, description="WebSocket heartbeat interval")
    WS_MESSAGE_TIMEOUT: int = Field(default=10, description="WebSocket message timeout")
    
    # Data Storage Configuration
    DATA_RETENTION_DAYS: int = Field(default=30, description="Days to retain sensor data")
    SENSOR_HISTORY_LIMIT: int = Field(default=1000, description="Maximum sensor history records")
    CLASSIFICATION_HISTORY_LIMIT: int = Field(default=500, description="Maximum classification records")
    
    # System Status Configuration
    STATUS_UPDATE_INTERVAL: int = Field(default=30, description="System status update interval")
    DEVICE_OFFLINE_THRESHOLD: int = Field(default=600, description="Device offline threshold in seconds")
    
    # Logging Configuration
    LOG_LEVEL: str = Field(default="INFO", description="Logging level")
    LOG_FORMAT: str = Field(
        default="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
        description="Log format"
    )
    
    # Security Configuration (for future use)
    SECRET_KEY: str = Field(
        default="your-secret-key-here-change-in-production",
        description="Secret key for JWT tokens"
    )
    ACCESS_TOKEN_EXPIRE_MINUTES: int = Field(default=30, description="Access token expiration")
    
    # Feature Flags
    ENABLE_WEBSOCKETS: bool = Field(default=True, description="Enable WebSocket support")
    ENABLE_MQTT: bool = Field(default=True, description="Enable MQTT support")
    ENABLE_AUTO_CLASSIFICATION: bool = Field(default=True, description="Enable automatic classification")
    ENABLE_DEVICE_REGISTRATION: bool = Field(default=True, description="Enable device auto-registration")
    
    # Performance Configuration
    MAX_CONCURRENT_REQUESTS: int = Field(default=50, description="Maximum concurrent requests")
    REQUEST_TIMEOUT: int = Field(default=30, description="Request timeout in seconds")
    
    class Config:
        env_file = ".env"
        env_file_encoding = "utf-8"
        case_sensitive = True

# Global settings instance
settings = Settings()

# Servo positions mapping
SERVO_POSITIONS = {
    "glass": 45,
    "plastic": 90, 
    "metal": 135
}

# Material display names
MATERIAL_DISPLAY_NAMES = {
    "glass": "Vidrio",
    "plastic": "Plástico", 
    "metal": "Metal"
}

# ESP32 Device Types
ESP32_DEVICE_TYPES = {
    "ESP32_CAM": "esp32-cam",
    "ESP32_CONTROL": "esp32-control"
}

# API Response Templates
API_RESPONSES = {
    "SUCCESS": {"status": "success", "message": "Operation completed successfully"},
    "ERROR": {"status": "error", "message": "An error occurred"},
    "NOT_FOUND": {"status": "not_found", "message": "Resource not found"},
    "OFFLINE": {"status": "offline", "message": "Device is offline"},
    "LOW_CONFIDENCE": {"status": "low_confidence", "message": "Classification confidence too low"}
}

def get_servo_position(material: str) -> int:
    """Get servo position for a material"""
    return SERVO_POSITIONS.get(material.lower(), 90)

def get_material_display_name(material: str) -> str:
    """Get display name for a material"""
    return MATERIAL_DISPLAY_NAMES.get(material.lower(), material.title())

def get_model_path(model_name: str = "") -> str:
    """Get full path to a model file"""
    if not model_name:
        return settings.MODEL_PATH
    return os.path.join(settings.MODEL_PATH, model_name)

def is_valid_material(material: str) -> bool:
    """Check if material is valid"""
    return material.lower() in [cls.lower() for cls in settings.MODEL_CLASSES]