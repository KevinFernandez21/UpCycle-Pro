from ai_client.aws_utils import post_json
import base64

def predecir_material():
    with open("ultima_imagen.jpg", "rb") as f:
        img_base64 = base64.b64encode(f.read()).decode()

    url = "https://mi-modelo-cnn.aws.com/predict"
    payload = {"imagen": img_base64}
    resultado = post_json(url, payload)

    return resultado.get("clase", "plastico")  # fallback
