
#include "Motor.h"

void Motor::speed(int val) {
  if (val > 0) {
    analogWrite(fpin, val);
    analogWrite(rpin, 0);
  }
  else {
    analogWrite(fpin, 0);
    analogWrite(rpin, -(val));
  }
}

//Motor BRmotor{10, 11}, FRmotor{8, 9}, BLmotor{14, 15}, FLmotor{12, 13};
Motor BRmotor{14, 15}, FRmotor{12, 13}, BLmotor{10, 11}, FLmotor{8, 9};
