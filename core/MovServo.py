#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time    
from adafruit_servokit import ServoKit   
#Constants
nbPCAServo=16 
#Parameters
MIN_IMP  =[500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500]
MAX_IMP  =[2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500]
MIN_ANG  =[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
MAX_ANG  =[180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180]
#Objects
pca = ServoKit(channels=16)
# function init 
def init():
    for i in range(nbPCAServo):
        pca.servo[i].set_pulse_width_range(MIN_IMP[i] , MAX_IMP[i])
# function main 


MyServo = 15

pca.servo[MyServo].angle = 0
time.sleep(.5)

pca.servo[MyServo].angle = 90
time.sleep(.5)

pca.servo[MyServo].angle = 180
time.sleep(.5)

pca.servo[MyServo].angle = 90
time.sleep(.5)

pca.servo[MyServo].angle = 0
time.sleep(.5)

while 1:
    pca.servo[MyServo].angle = 10
    time.sleep(.3)

    pca.servo[MyServo].angle = 45
    time.sleep(.3)

    pca.servo[MyServo].angle = 90
    time.sleep(.3)

    pca.servo[MyServo].angle = 145
    time.sleep(.3)

    pca.servo[MyServo].angle = 90
    time.sleep(.3)

    pca.servo[MyServo].angle = 45
    time.sleep(.3)
