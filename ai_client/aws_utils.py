import requests

def post_json(url, payload, headers=None):
    try:
        response = requests.post(url, json=payload, headers=headers)
        response.raise_for_status()
        return response.json()
    except Exception as e:
        print(f"[ERROR AWS] {e}")
        return {"error": str(e)}
