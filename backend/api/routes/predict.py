from fastapi import APIRouter
from ai_client.aws_cnn import predecir_material

router = APIRouter(prefix="/predict")

@router.get("/")
def get_prediction():
    clase = predecir_material()
    return {"clase": clase}
