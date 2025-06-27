from fastapi import FastAPI
from api.routes import predict, control, weight

app = FastAPI()

app.include_router(predict.router)
app.include_router(control.router)
app.include_router(weight.router)
