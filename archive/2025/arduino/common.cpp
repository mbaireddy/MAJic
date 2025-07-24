#include <Arduino.h>
#include "common.h"
#include "Motor.h"

int fleft;
int fright;
int bright;
int bleft;
int result;

int cmap(int val, int olow, int ohigh, int mlow, int mhigh) {
  return constrain(map(val, olow, ohigh, mlow, mhigh), mlow, mhigh);
}

void tcaselect(uint8_t i) {
  if (i > 3) return;

  Serial.println(i);

  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);

  int ret = Wire.endTransmission();

  if (ret != 0 && ret != 5) {
    Serial.print("Mux error: ");
    Serial.println(ret);
    while (1) delay(10);
  }
}

void speedSet(int mspeed) {
  FRmotor.speed(mspeed);
  FLmotor.speed(mspeed);
  BRmotor.speed(mspeed);
  BLmotor.speed(mspeed);
}

void parseMotors(){
  fleft = Serial1.parseInt();
  fright = Serial1.parseInt();
  result = Serial1.parseInt();
  bleft = fleft;
  bright = fright;
  Serial.print("fleft: ");
  Serial.println(fleft);
  Serial.print("fright: ");
  Serial.println(fright);
  
  FLmotor.speed(fleft);
  BLmotor.speed(bleft);
  FRmotor.speed(fright);
  BRmotor.speed(bright);
}
