from fastapi import WebSocket, WebSocketDisconnect
from typing import Dict, List, Set
import json
import logging
import asyncio
from datetime import datetime

logger = logging.getLogger(__name__)

class WebSocketManager:
    def __init__(self):
        # Conexiones activas por tipo de cliente
        self.active_connections: Dict[str, List[WebSocket]] = {
            "flutter_app": [],    # Aplicación Flutter
            "web_dashboard": [],  # Dashboard web (si existe)
            "node_red": [],       # Node-RED
            "esp32_devices": []   # Dispositivos ESP32
        }
        
        # Suscripciones a eventos específicos
        self.event_subscriptions: Dict[str, Set[WebSocket]] = {
            "sensor_data": set(),
            "camera_stream": set(), 
            "classification_results": set(),
            "system_status": set(),
            "device_status": set()
        }
        
        # Historial reciente para nuevas conexiones
        self.recent_data = {
            "sensor_data": [],
            "classification_results": [],
            "system_status": None,
            "device_status": []
        }
        
    async def connect(self, websocket: WebSocket, client_type: str = "flutter_app"):
        """Aceptar nueva conexión WebSocket"""
        await websocket.accept()
        
        if client_type in self.active_connections:
            self.active_connections[client_type].append(websocket)
            logger.info(f"New WebSocket connection: {client_type}, total: {len(self.active_connections[client_type])}")
            
            # Enviar datos recientes al nuevo cliente
            await self.send_recent_data_to_client(websocket)
        else:
            logger.warning(f"Unknown client type: {client_type}")
            self.active_connections["flutter_app"].append(websocket)
    
    async def disconnect(self, websocket: WebSocket, client_type: str = "flutter_app"):
        """Desconectar WebSocket"""
        if client_type in self.active_connections:
            if websocket in self.active_connections[client_type]:
                self.active_connections[client_type].remove(websocket)
                logger.info(f"WebSocket disconnected: {client_type}, remaining: {len(self.active_connections[client_type])}")
        
        # Remover de todas las suscripciones
        for event_type in self.event_subscriptions:
            self.event_subscriptions[event_type].discard(websocket)
    
    async def subscribe_to_event(self, websocket: WebSocket, event_type: str):
        """Suscribir WebSocket a eventos específicos"""
        if event_type in self.event_subscriptions:
            self.event_subscriptions[event_type].add(websocket)
            logger.info(f"WebSocket subscribed to {event_type}, subscribers: {len(self.event_subscriptions[event_type])}")
            
            # Enviar confirmación de suscripción
            await self.send_to_websocket(websocket, {
                "type": "subscription_confirmed",
                "event_type": event_type,
                "timestamp": datetime.now().isoformat()
            })
    
    async def unsubscribe_from_event(self, websocket: WebSocket, event_type: str):
        """Desuscribir WebSocket de eventos específicos"""
        if event_type in self.event_subscriptions:
            self.event_subscriptions[event_type].discard(websocket)
            logger.info(f"WebSocket unsubscribed from {event_type}")
    
    async def send_to_websocket(self, websocket: WebSocket, data: dict):
        """Enviar datos a un WebSocket específico"""
        try:
            await websocket.send_text(json.dumps(data))
        except Exception as e:
            logger.error(f"Error sending to websocket: {e}")
    
    async def broadcast_to_type(self, client_type: str, data: dict):
        """Enviar datos a todos los clientes de un tipo específico"""
        if client_type not in self.active_connections:
            return
        
        disconnected = []
        for websocket in self.active_connections[client_type]:
            try:
                await websocket.send_text(json.dumps(data))
            except Exception as e:
                logger.error(f"Error broadcasting to {client_type}: {e}")
                disconnected.append(websocket)
        
        # Limpiar conexiones desconectadas
        for websocket in disconnected:
            self.active_connections[client_type].remove(websocket)
    
    async def broadcast_to_event_subscribers(self, event_type: str, data: dict):
        """Enviar datos a todos los suscriptores de un evento específico"""
        if event_type not in self.event_subscriptions:
            return
        
        # Agregar metadatos del evento
        event_data = {
            "event_type": event_type,
            "timestamp": datetime.now().isoformat(),
            "data": data
        }
        
        disconnected = set()
        for websocket in self.event_subscriptions[event_type]:
            try:
                await websocket.send_text(json.dumps(event_data))
            except Exception as e:
                logger.error(f"Error broadcasting event {event_type}: {e}")
                disconnected.add(websocket)
        
        # Limpiar conexiones desconectadas
        self.event_subscriptions[event_type] -= disconnected
        
        # Guardar en historial reciente
        await self.save_to_recent_data(event_type, data)
    
    async def save_to_recent_data(self, event_type: str, data: dict):
        """Guardar datos en historial reciente"""
        if event_type in self.recent_data:
            if isinstance(self.recent_data[event_type], list):
                self.recent_data[event_type].append(data)
                
                # Mantener solo los últimos 50 elementos
                if len(self.recent_data[event_type]) > 50:
                    self.recent_data[event_type].pop(0)
            else:
                # Para datos únicos (como system_status)
                self.recent_data[event_type] = data
    
    async def send_recent_data_to_client(self, websocket: WebSocket):
        """Enviar datos recientes a un nuevo cliente"""
        try:
            # Enviar estado del sistema
            if self.recent_data["system_status"]:
                await self.send_to_websocket(websocket, {
                    "event_type": "system_status",
                    "timestamp": datetime.now().isoformat(),
                    "data": self.recent_data["system_status"]
                })
            
            # Enviar últimos datos de sensores
            if self.recent_data["sensor_data"]:
                await self.send_to_websocket(websocket, {
                    "event_type": "sensor_data",
                    "timestamp": datetime.now().isoformat(),
                    "data": self.recent_data["sensor_data"][-1]  # Solo el más reciente
                })
            
            # Enviar últimas clasificaciones
            if self.recent_data["classification_results"]:
                for result in self.recent_data["classification_results"][-5:]:  # Últimas 5
                    await self.send_to_websocket(websocket, {
                        "event_type": "classification_results",
                        "timestamp": datetime.now().isoformat(),
                        "data": result
                    })
                    
        except Exception as e:
            logger.error(f"Error sending recent data to client: {e}")
    
    async def broadcast_sensor_data(self, sensor_data: dict):
        """Enviar datos de sensores a suscriptores"""
        await self.broadcast_to_event_subscribers("sensor_data", sensor_data)
    
    async def broadcast_classification_result(self, classification: dict):
        """Enviar resultado de clasificación a suscriptores"""
        await self.broadcast_to_event_subscribers("classification_results", classification)
    
    async def broadcast_system_status(self, status: dict):
        """Enviar estado del sistema a suscriptores"""
        await self.broadcast_to_event_subscribers("system_status", status)
    
    async def broadcast_device_status(self, device_status: dict):
        """Enviar estado de dispositivos a suscriptores"""
        await self.broadcast_to_event_subscribers("device_status", device_status)
    
    async def broadcast_camera_frame(self, frame_data: dict):
        """Enviar frame de cámara a suscriptores (base64 encoded)"""
        await self.broadcast_to_event_subscribers("camera_stream", frame_data)
    
    def get_connection_stats(self) -> dict:
        """Obtener estadísticas de conexiones"""
        stats = {
            "total_connections": sum(len(conns) for conns in self.active_connections.values()),
            "connections_by_type": {
                client_type: len(connections)
                for client_type, connections in self.active_connections.items()
            },
            "event_subscriptions": {
                event_type: len(subscribers)
                for event_type, subscribers in self.event_subscriptions.items()
            },
            "recent_data_counts": {
                data_type: len(data) if isinstance(data, list) else (1 if data else 0)
                for data_type, data in self.recent_data.items()
            }
        }
        return stats
    
    async def handle_client_message(self, websocket: WebSocket, message: dict):
        """Manejar mensajes de clientes"""
        try:
            message_type = message.get("type", "unknown")
            
            if message_type == "subscribe":
                event_type = message.get("event_type")
                if event_type:
                    await self.subscribe_to_event(websocket, event_type)
            
            elif message_type == "unsubscribe":
                event_type = message.get("event_type")
                if event_type:
                    await self.unsubscribe_from_event(websocket, event_type)
            
            elif message_type == "ping":
                # Responder pong para keep-alive
                await self.send_to_websocket(websocket, {
                    "type": "pong",
                    "timestamp": datetime.now().isoformat()
                })
            
            elif message_type == "get_stats":
                # Enviar estadísticas de conexión
                stats = self.get_connection_stats()
                await self.send_to_websocket(websocket, {
                    "type": "connection_stats",
                    "data": stats,
                    "timestamp": datetime.now().isoformat()
                })
            
            elif message_type == "esp32_command":
                # Comando desde app Flutter para ESP32
                device_id = message.get("device_id")
                command = message.get("command")
                parameters = message.get("parameters", {})
                
                # Reenviar a dispositivos ESP32 conectados
                command_data = {
                    "type": "command",
                    "device_id": device_id,
                    "command": command,
                    "parameters": parameters,
                    "source": "flutter_app",
                    "timestamp": datetime.now().isoformat()
                }
                
                await self.broadcast_to_type("esp32_devices", command_data)
                logger.info(f"Command forwarded to ESP32 devices: {command} for {device_id}")
            
            else:
                logger.warning(f"Unknown message type: {message_type}")
                
        except Exception as e:
            logger.error(f"Error handling client message: {e}")

# Instancia global del administrador WebSocket
websocket_manager = WebSocketManager()