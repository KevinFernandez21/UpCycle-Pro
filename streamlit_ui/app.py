import streamlit as st
import requests

st.title('Dashboard Sistema de Clasificación de Residuos')

# Endpoint de la API de la Raspberry Pi
api_url = "http://localhost:8000"

# Obtener el estado del sistema
estado = requests.get(f"{api_url}/control/status").json()
if estado['activo']:
    st.success('Sistema activo')
else:
    st.error('Sistema detenido')

# Mostrar peso actual
peso = requests.get(f"{api_url}/weight").json()
st.write(f"Peso del material: {peso['peso']} kg")

# Predicción actual
prediccion = requests.get(f"{api_url}/predict").json()
st.write(f"Material predicho: {prediccion['clase']}")

# Botón para controlar el sistema
if st.button('Detener Sistema'):
    requests.post(f"{api_url}/control/stop")
    st.warning('Sistema detenido')

if st.button('Iniciar Sistema'):
    requests.post(f"{api_url}/control/start")
    st.success('Sistema iniciado')
