from fastapi import APIRouter
from core.sensor_hx711 import Balanza

router = APIRouter(prefix="/weight")
balanza = Balanza()

@router.get("/")
def get_peso():
    return {"peso": balanza.leer_peso()}
