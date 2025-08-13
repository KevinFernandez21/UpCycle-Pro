from fastapi import APIRouter

router = APIRouter(prefix="/control")

estado_sistema = {"activo": True}

@router.post("/start")
def start():
    estado_sistema["activo"] = True
    return {"estado": "iniciado"}

@router.post("/stop")
def stop():
    estado_sistema["activo"] = False
    return {"estado": "detenido"}

@router.get("/status")
def status():
    return {"activo": estado_sistema["activo"]}
