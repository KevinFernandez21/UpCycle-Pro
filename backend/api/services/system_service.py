import aiohttp
import asyncio
import logging
from typing import Dict, List, Optional, Tuple
from datetime import datetime
import json
import io
from PIL import Image
import numpy as np

logger = logging.getLogger(__name__)

class SystemService:
    def __init__(self):
        self.esp32_cam_ips = []  # Se llenarán dinámicamente
        self.esp32_control_ips = []
        self.default_timeout = 10
        self.retry_attempts = 3
        
    async def discover_esp32_devices(self) -> Dict[str, List[str]]:
        """Descubrir dispositivos ESP32 en la red local"""
        discovered = {
            "esp32_cam": [],
            "esp32_control": []
        }
        
        # Rango de IPs común para dispositivos locales
        base_ip = "192.168.1."  # Ajustar según la red
        
        async def check_device(ip: str):
            try:
                timeout = aiohttp.ClientTimeout(total=2)
                async with aiohttp.ClientSession(timeout=timeout) as session:
                    async with session.get(f"http://{ip}/status") as response:
                        if response.status == 200:
                            data = await response.json()
                            device_type = data.get("device", "unknown")
                            
                            if "cam" in device_type.lower():
                                discovered["esp32_cam"].append(ip)
                            elif "control" in device_type.lower():
                                discovered["esp32_control"].append(ip)
                                
            except Exception:
                pass  # Dispositivo no disponible
        
        # Verificar rango de IPs común
        tasks = []
        for i in range(1, 255):
            ip = f"{base_ip}{i}"
            tasks.append(check_device(ip))
            
        await asyncio.gather(*tasks, return_exceptions=True)
        
        # Actualizar listas internas
        self.esp32_cam_ips = discovered["esp32_cam"]
        self.esp32_control_ips = discovered["esp32_control"]
        
        logger.info(f"Discovered devices - CAM: {len(self.esp32_cam_ips)}, Control: {len(self.esp32_control_ips)}")
        
        return discovered
    
    async def capture_image_from_esp32(self, device_ip: Optional[str] = None) -> Optional[bytes]:
        """Capturar imagen desde ESP32-CAM"""
        if not device_ip:
            if not self.esp32_cam_ips:
                await self.discover_esp32_devices()
            device_ip = self.esp32_cam_ips[0] if self.esp32_cam_ips else None
            
        if not device_ip:
            logger.error("No ESP32-CAM devices found")
            return None
        
        for attempt in range(self.retry_attempts):
            try:
                timeout = aiohttp.ClientTimeout(total=self.default_timeout)
                async with aiohttp.ClientSession(timeout=timeout) as session:
                    async with session.get(f"http://{device_ip}/capture") as response:
                        if response.status == 200:
                            image_data = await response.read()
                            logger.info(f"Image captured from {device_ip}, size: {len(image_data)} bytes")
                            return image_data
                        else:
                            logger.warning(f"HTTP {response.status} from {device_ip}/capture")
                            
            except asyncio.TimeoutError:
                logger.warning(f"Timeout capturing from {device_ip}, attempt {attempt + 1}")
            except Exception as e:
                logger.error(f"Error capturing from {device_ip}: {e}")
                
        return None
    
    async def control_esp32_cam(self, 
                              device_ip: Optional[str] = None,
                              flash_enabled: Optional[bool] = None,
                              quality: Optional[int] = None,
                              action: Optional[str] = None) -> bool:
        """Controlar ESP32-CAM (flash, calidad, reinicio)"""
        if not device_ip:
            if not self.esp32_cam_ips:
                await self.discover_esp32_devices()
            device_ip = self.esp32_cam_ips[0] if self.esp32_cam_ips else None
            
        if not device_ip:
            logger.error("No ESP32-CAM devices found")
            return False
        
        try:
            control_data = {}
            endpoint = "/control"
            
            if action == "restart":
                endpoint = "/restart"
            else:
                if flash_enabled is not None:
                    control_data["flash"] = flash_enabled
                if quality is not None:
                    control_data["quality"] = max(0, min(63, quality))
            
            timeout = aiohttp.ClientTimeout(total=self.default_timeout)
            async with aiohttp.ClientSession(timeout=timeout) as session:
                async with session.post(
                    f"http://{device_ip}{endpoint}",
                    json=control_data
                ) as response:
                    success = response.status == 200
                    if success:
                        logger.info(f"ESP32-CAM control successful: {action or 'settings_update'}")
                    else:
                        logger.warning(f"ESP32-CAM control failed: HTTP {response.status}")
                    return success
                    
        except Exception as e:
            logger.error(f"Error controlling ESP32-CAM {device_ip}: {e}")
            return False
    
    async def get_sensor_data(self, device_ip: Optional[str] = None) -> Optional[Dict]:
        """Obtener datos de sensores desde ESP32-CONTROL"""
        if not device_ip:
            if not self.esp32_control_ips:
                await self.discover_esp32_devices()
            device_ip = self.esp32_control_ips[0] if self.esp32_control_ips else None
            
        if not device_ip:
            logger.error("No ESP32-CONTROL devices found")
            return None
        
        try:
            timeout = aiohttp.ClientTimeout(total=5)
            async with aiohttp.ClientSession(timeout=timeout) as session:
                async with session.get(f"http://{device_ip}/sensors") as response:
                    if response.status == 200:
                        sensor_data = await response.json()
                        logger.info(f"Sensor data received from {device_ip}")
                        return sensor_data
                    else:
                        logger.warning(f"Failed to get sensor data: HTTP {response.status}")
                        
        except Exception as e:
            logger.error(f"Error getting sensor data from {device_ip}: {e}")
            
        return None
    
    async def send_classification_command(self, 
                                        device_ip: Optional[str] = None,
                                        material: str = "plastic",
                                        servo_position: int = 90) -> bool:
        """Enviar comando de clasificación a ESP32-CONTROL"""
        if not device_ip:
            if not self.esp32_control_ips:
                await self.discover_esp32_devices()
            device_ip = self.esp32_control_ips[0] if self.esp32_control_ips else None
            
        if not device_ip:
            logger.error("No ESP32-CONTROL devices found")
            return False
        
        try:
            command_data = {
                "action": "classify",
                "material": material,
                "servo_position": servo_position
            }
            
            timeout = aiohttp.ClientTimeout(total=10)
            async with aiohttp.ClientSession(timeout=timeout) as session:
                async with session.post(
                    f"http://{device_ip}/classify",
                    json=command_data
                ) as response:
                    success = response.status == 200
                    if success:
                        logger.info(f"Classification command sent: {material} -> {servo_position}°")
                    else:
                        logger.warning(f"Classification command failed: HTTP {response.status}")
                    return success
                    
        except Exception as e:
            logger.error(f"Error sending classification command: {e}")
            return False
    
    async def get_system_metrics(self) -> Dict:
        """Obtener métricas del sistema completo"""
        metrics = {
            "timestamp": datetime.now().isoformat(),
            "devices": {
                "esp32_cam": {
                    "discovered": len(self.esp32_cam_ips),
                    "ips": self.esp32_cam_ips,
                    "status": []
                },
                "esp32_control": {
                    "discovered": len(self.esp32_control_ips),
                    "ips": self.esp32_control_ips,
                    "status": []
                }
            },
            "connectivity": {
                "total_devices": len(self.esp32_cam_ips) + len(self.esp32_control_ips),
                "online_devices": 0,
                "last_discovery": datetime.now().isoformat()
            }
        }
        
        # Verificar estado de dispositivos
        online_count = 0
        
        # Verificar ESP32-CAM
        for ip in self.esp32_cam_ips:
            try:
                timeout = aiohttp.ClientTimeout(total=3)
                async with aiohttp.ClientSession(timeout=timeout) as session:
                    async with session.get(f"http://{ip}/status") as response:
                        if response.status == 200:
                            data = await response.json()
                            metrics["devices"]["esp32_cam"]["status"].append({
                                "ip": ip,
                                "status": "online",
                                "info": data
                            })
                            online_count += 1
                        else:
                            metrics["devices"]["esp32_cam"]["status"].append({
                                "ip": ip,
                                "status": "error",
                                "http_code": response.status
                            })
            except Exception as e:
                metrics["devices"]["esp32_cam"]["status"].append({
                    "ip": ip,
                    "status": "offline",
                    "error": str(e)
                })
        
        # Verificar ESP32-CONTROL
        for ip in self.esp32_control_ips:
            try:
                timeout = aiohttp.ClientTimeout(total=3)
                async with aiohttp.ClientSession(timeout=timeout) as session:
                    async with session.get(f"http://{ip}/status") as response:
                        if response.status == 200:
                            data = await response.json()
                            metrics["devices"]["esp32_control"]["status"].append({
                                "ip": ip,
                                "status": "online",
                                "info": data
                            })
                            online_count += 1
                        else:
                            metrics["devices"]["esp32_control"]["status"].append({
                                "ip": ip,
                                "status": "error",
                                "http_code": response.status
                            })
            except Exception as e:
                metrics["devices"]["esp32_control"]["status"].append({
                    "ip": ip,
                    "status": "offline",
                    "error": str(e)
                })
        
        metrics["connectivity"]["online_devices"] = online_count
        
        return metrics
    
    def preprocess_image_for_classification(self, image_data: bytes) -> Optional[np.ndarray]:
        """Preprocesar imagen para clasificación CNN"""
        try:
            image = Image.open(io.BytesIO(image_data))
            if image.mode != 'RGB':
                image = image.convert('RGB')
                
            # Redimensionar a 224x224 para el modelo CNN
            image = image.resize((224, 224))
            
            # Convertir a numpy array y normalizar
            img_array = np.array(image)
            img_array = img_array.astype('float32') / 255.0
            img_array = np.expand_dims(img_array, axis=0)
            
            return img_array
            
        except Exception as e:
            logger.error(f"Error preprocessing image: {e}")
            return None
    
    async def classify_image(self, image_data: bytes, model=None) -> Optional[Dict]:
        """Clasificar imagen usando modelo CNN"""
        if model is None:
            logger.error("No model provided for classification")
            return None
            
        try:
            processed_image = self.preprocess_image_for_classification(image_data)
            if processed_image is None:
                return None
            
            # Realizar predicción
            predictions = model.predict(processed_image)
            predicted_class_idx = int(np.argmax(predictions[0]))
            
            class_names = ['glass', 'metal', 'plastic']  # Orden del modelo
            predicted_class = class_names[predicted_class_idx]
            confidence = float(predictions[0][predicted_class_idx])
            
            # Mapear posiciones de servo
            servo_positions = {
                'glass': 45,
                'plastic': 90,
                'metal': 135
            }
            
            result = {
                "predicted_class": predicted_class,
                "confidence": confidence,
                "confidence_percentage": f"{confidence * 100:.1f}%",
                "all_probabilities": {
                    class_names[i]: float(predictions[0][i])
                    for i in range(len(class_names))
                },
                "servo_position": servo_positions.get(predicted_class, 90),
                "material_type": predicted_class.title(),
                "timestamp": datetime.now().isoformat()
            }
            
            logger.info(f"Image classified as {predicted_class} with {confidence:.3f} confidence")
            
            return result
            
        except Exception as e:
            logger.error(f"Error in image classification: {e}")
            return None