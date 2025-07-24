#pragma once

//#include "CytronMotorDriver.h"
//#include "turn.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <utility/imumaths.h>
#include <string>
#include <Adafruit_SSD1306.h>
#include "Adafruit_APDS9960.h"
#include <VL53L0X.h>
#include <Servo.h>

#define TCAADDR 0x70
#define color 0
#define bottom 1
//#define top 2
//#define side 3

extern VL53L0X tof;
extern Adafruit_APDS9960 apds;
extern Servo camservo;
extern Servo evacservo;
extern int fleft;
extern int fright;
extern int bright;
extern int bleft;
extern int result;

extern Adafruit_BNO055 bno;
extern sensors_event_t event;


/*
CytronMD BRmotor(PWM_DIR, 8, 9);
CytronMD FRmotor(PWM_DIR, 10, 11);
CytronMD BLmotor(PWM_DIR, 12, 13);
CytronMD FLmotor(PWM_DIR, 14, 15);*/

int cmap(int val, int olow, int ohigh, int mlow, int mhigh);
void tcaselect(uint8_t i);
void speedSet(int mspeed);
void parseMotors();
