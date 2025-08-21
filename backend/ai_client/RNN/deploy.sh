#!/bin/bash

# Script de despliegue para RNN de predicción de volúmenes en AWS

echo "🚀 Iniciando despliegue de RNN UpCycle Pro en AWS..."

# Verificar prerequisitos
echo "📋 Verificando prerequisitos..."

# Verificar Node.js y npm
if ! command -v node &> /dev/null; then
    echo "❌ Node.js no está instalado. Instálalo desde https://nodejs.org/"
    exit 1
fi

# Verificar Serverless Framework
if ! command -v serverless &> /dev/null; then
    echo "📦 Instalando Serverless Framework..."
    npm install -g serverless
fi

# Verificar AWS CLI
if ! command -v aws &> /dev/null; then
    echo "❌ AWS CLI no está instalado. Instálalo desde https://aws.amazon.com/cli/"
    exit 1
fi

# Verificar credenciales AWS
echo "🔑 Verificando credenciales AWS..."
if ! aws sts get-caller-identity &> /dev/null; then
    echo "❌ Credenciales AWS no configuradas. Ejecuta 'aws configure'"
    exit 1
fi

# Instalar dependencias de Serverless
echo "📦 Instalando plugins de Serverless..."
npm init -y 2>/dev/null || true
npm install --save-dev serverless-python-requirements

# Verificar que los archivos necesarios existen
echo "📁 Verificando archivos necesarios..."
required_files=("aws_rnn_service.py" "data_ingestion.py" "serverless.yml" "requirements.txt")
for file in "${required_files[@]}"; do
    if [ ! -f "$file" ]; then
        echo "❌ Archivo faltante: $file"
        exit 1
    fi
done

# Configurar stage (desarrollo o producción)
STAGE=${1:-dev}
echo "🎯 Desplegando en stage: $STAGE"

# Desplegar con Serverless
echo "☁️ Desplegando servicios AWS..."
serverless deploy --stage $STAGE --verbose

# Verificar despliegue
if [ $? -eq 0 ]; then
    echo "✅ Despliegue exitoso!"
    
    # Obtener endpoints
    echo ""
    echo "📍 Endpoints desplegados:"
    serverless info --stage $STAGE
    
    echo ""
    echo "🔧 Próximos pasos:"
    echo "1. Actualiza backend/configs/aws.yaml con el endpoint de API Gateway"
    echo "2. Configura las credenciales AWS en el backend"
    echo "3. Prueba los endpoints desde /docs en tu API FastAPI"
    
    echo ""
    echo "📚 Endpoints disponibles:"
    echo "- POST /rnn/ingest-volume - Enviar datos de volumen"
    echo "- POST /rnn/predict-volume - Obtener predicciones"
    echo "- GET /rnn/daily-summary - Resumen diario"
    echo "- GET /rnn/config-status - Estado de configuración"
    echo "- POST /rnn/test-connection - Probar conexión AWS"
    
else
    echo "❌ Error en el despliegue"
    exit 1
fi

echo ""
echo "🎉 ¡Despliegue completado!"