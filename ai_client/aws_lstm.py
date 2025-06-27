from ai_client.aws_utils import post_json

def enviar_peso(clase, peso):
    url = "https://mi-modelo-lstm.aws.com/register"
    payload = {
        "tipo_material": clase,
        "peso": peso
    }
    post_json(url, payload)
