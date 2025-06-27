from hx711 import HX711

class Balanza:
    def __init__(self, dout=21, pd_sck=20):
        self.hx = HX711(dout_pin=dout, pd_sck_pin=pd_sck)
        self.hx.zero()
        self.hx.set_scale_ratio(7050)

    def leer_peso(self):
        return round(self.hx.get_weight_mean(20), 2)
