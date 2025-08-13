#!/usr/bin/env python3
"""
Dashboard de Streamlit para UpCycle Pro
Sistema de clasificación de materiales con CNN
"""

import streamlit as st
import requests
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go
from datetime import datetime, timedelta
import time
import json
from PIL import Image
import io

# Configuración de la página
st.set_page_config(
    page_title="UpCycle Pro Dashboard",
    page_icon="♻️",
    layout="wide",
    initial_sidebar_state="expanded"
)

# Configuración de la API
API_BASE_URL = "http://localhost:8000"

def check_api_status():
    """Verificar si la API está disponible"""
    try:
        response = requests.get(f"{API_BASE_URL}/health", timeout=5)
        return response.status_code == 200, response.json() if response.status_code == 200 else None
    except Exception as e:
        return False, str(e)

def get_available_models():
    """Obtener modelos disponibles"""
    try:
        response = requests.get(f"{API_BASE_URL}/available_models", timeout=10)
        if response.status_code == 200:
            return response.json()
        return None
    except Exception as e:
        st.error(f"Error obteniendo modelos: {e}")
        return None

def load_model(model_name):
    """Cargar un modelo específico"""
    try:
        response = requests.post(f"{API_BASE_URL}/load_model", params={"model_name": model_name}, timeout=30)
        return response.status_code == 200, response.json()
    except Exception as e:
        return False, str(e)

def get_model_info():
    """Obtener información del modelo actual"""
    try:
        response = requests.get(f"{API_BASE_URL}/model_info", timeout=10)
        if response.status_code == 200:
            return response.json()
        return None
    except Exception as e:
        return None

def predict_image(image_file):
    """Enviar imagen para predicción"""
    try:
        files = {"file": image_file}
        response = requests.post(f"{API_BASE_URL}/predict", files=files, timeout=30)
        if response.status_code == 200:
            return response.json()
        return None
    except Exception as e:
        st.error(f"Error en predicción: {e}")
        return None

def main():
    # Título principal
    st.title("♻️ UpCycle Pro Dashboard")
    st.markdown("**Sistema de Clasificación Inteligente de Materiales**")
    
    # Sidebar
    st.sidebar.title("🎛️ Control Panel")
    
    # Verificar estado de la API
    api_online, api_data = check_api_status()
    
    if api_online:
        st.sidebar.success("✅ API Conectada")
        if api_data and 'model_loaded' in api_data:
            model_status = "🤖 Modelo Cargado" if api_data['model_loaded'] else "⚠️ Modelo No Cargado"
            st.sidebar.info(model_status)
    else:
        st.sidebar.error("❌ API Desconectada")
        st.error("⚠️ No se puede conectar con la API de FastAPI. Asegúrate de que esté ejecutándose en http://localhost:8000")
        st.code("python start_api.py")
        return
    
    # Tabs principales
    tab1, tab2, tab3, tab4 = st.tabs(["🏠 Dashboard", "🤖 Modelos", "📸 Predicción", "📊 Estadísticas"])
    
    with tab1:
        st.header("Estado del Sistema")
        
        # Métricas principales
        col1, col2, col3, col4 = st.columns(4)
        
        with col1:
            st.metric("Estado API", "🟢 Online" if api_online else "🔴 Offline")
        
        with col2:
            model_loaded = api_data.get('model_loaded', False) if api_data else False
            st.metric("Modelo CNN", "✅ Cargado" if model_loaded else "❌ No Cargado")
        
        with col3:
            classes = api_data.get('classes', []) if api_data else []
            st.metric("Clases", len(classes))
        
        with col4:
            st.metric("Última Actualización", datetime.now().strftime("%H:%M:%S"))
        
        # Información del sistema
        if api_data:
            st.subheader("Información del Sistema")
            info_data = {
                "Versión": api_data.get('version', 'N/A'),
                "Estado": api_data.get('status', 'N/A'),
                "Clases Soportadas": ', '.join(classes) if classes else 'N/A',
                "Timestamp": api_data.get('timestamp', 'N/A')
            }
            
            df_info = pd.DataFrame(list(info_data.items()), columns=['Parámetro', 'Valor'])
            st.dataframe(df_info, use_container_width=True)
    
    with tab2:
        st.header("Gestión de Modelos CNN")
        
        # Obtener modelos disponibles
        models_data = get_available_models()
        
        if models_data:
            st.subheader("Modelos Disponibles")
            
            # Tabla de modelos
            if models_data.get('models'):
                models_df = pd.DataFrame(models_data['models'])
                st.dataframe(models_df, use_container_width=True)
                
                # Selector de modelo
                model_names = [model['filename'] for model in models_data['models']]
                selected_model = st.selectbox("Seleccionar Modelo:", model_names)
                
                col1, col2 = st.columns([1, 1])
                
                with col1:
                    if st.button("🔄 Cargar Modelo", type="primary"):
                        with st.spinner(f"Cargando {selected_model}..."):
                            success, result = load_model(selected_model)
                            if success:
                                st.success(f"✅ Modelo {selected_model} cargado exitosamente!")
                                st.json(result)
                            else:
                                st.error(f"❌ Error cargando modelo: {result}")
                
                with col2:
                    if st.button("ℹ️ Info del Modelo Actual"):
                        model_info = get_model_info()
                        if model_info:
                            st.json(model_info)
                        else:
                            st.warning("No hay modelo cargado")
            else:
                st.warning("No se encontraron modelos en la carpeta CNN")
        else:
            st.error("Error obteniendo información de modelos")
    
    with tab3:
        st.header("Predicción de Materiales")
        
        # Upload de imagen
        uploaded_file = st.file_uploader(
            "Selecciona una imagen de material:",
            type=['png', 'jpg', 'jpeg'],
            help="Sube una imagen de vidrio, metal o plástico para clasificar"
        )
        
        if uploaded_file is not None:
            # Mostrar imagen
            col1, col2 = st.columns([1, 1])
            
            with col1:
                st.subheader("Imagen Original")
                image = Image.open(uploaded_file)
                st.image(image, caption="Imagen a clasificar", use_container_width=True)
            
            with col2:
                st.subheader("Resultado de Clasificación")
                
                if st.button("🔍 Clasificar Material", type="primary"):
                    # Reset file pointer
                    uploaded_file.seek(0)
                    
                    with st.spinner("Clasificando imagen..."):
                        result = predict_image(uploaded_file)
                        
                        if result:
                            # Resultado principal
                            predicted_class = result.get('predicted_class', 'N/A')
                            confidence = result.get('confidence', 0)
                            
                            # Métricas del resultado
                            st.metric("Material Detectado", predicted_class)
                            st.metric("Confianza", f"{confidence:.2%}")
                            
                            # Gráfico de probabilidades
                            if 'all_probabilities' in result:
                                probs = result['all_probabilities']
                                
                                # Crear gráfico de barras
                                df_probs = pd.DataFrame(
                                    list(probs.items()),
                                    columns=['Material', 'Probabilidad']
                                )
                                
                                fig = px.bar(
                                    df_probs,
                                    x='Material',
                                    y='Probabilidad',
                                    title="Probabilidades por Clase",
                                    color='Probabilidad',
                                    color_continuous_scale='viridis'
                                )
                                fig.update_layout(height=400)
                                st.plotly_chart(fig, use_container_width=True)
                            
                            # Detalles adicionales
                            with st.expander("Detalles de la Predicción"):
                                st.json(result)
                        else:
                            st.error("❌ Error en la clasificación")
    
    with tab4:
        st.header("Estadísticas del Sistema")
        
        # Simulación de datos estadísticos
        st.subheader("Clasificaciones por Material (Última Semana)")
        
        # Datos simulados para demo
        materials_data = {
            'Material': ['Plástico', 'Vidrio', 'Metal'],
            'Cantidad': [45, 32, 23],
            'Porcentaje': [45, 32, 23]
        }
        
        df_materials = pd.DataFrame(materials_data)
        
        col1, col2 = st.columns([1, 1])
        
        with col1:
            # Gráfico de barras
            fig_bar = px.bar(
                df_materials,
                x='Material',
                y='Cantidad',
                title="Cantidad por Material",
                color='Material'
            )
            st.plotly_chart(fig_bar, use_container_width=True)
        
        with col2:
            # Gráfico de pie
            fig_pie = px.pie(
                df_materials,
                values='Cantidad',
                names='Material',
                title="Distribución de Materiales"
            )
            st.plotly_chart(fig_pie, use_container_width=True)
        
        # Tabla de estadísticas
        st.subheader("Resumen Estadístico")
        st.dataframe(df_materials, use_container_width=True)
        
        # Métricas de eficiencia
        st.subheader("Métricas de Rendimiento")
        col1, col2, col3 = st.columns(3)
        
        with col1:
            st.metric("Precisión Promedio", "94.5%", "2.1%")
        
        with col2:
            st.metric("Tiempo Promedio", "1.2s", "-0.3s")
        
        with col3:
            st.metric("Items Procesados", "2,847", "156")

if __name__ == "__main__":
    main()