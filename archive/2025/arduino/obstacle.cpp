#include "common.h"
#include "turn.h"
#include "Motor.h"

//TRIG is 26
//ECHO is 19

float ultrasonic() {
  digitalWrite(26, LOW);
  delayMicroseconds(2);
  digitalWrite(26, HIGH);
  delayMicroseconds(10);
  digitalWrite(26, LOW);
  //wait for it to come back
  float duration = pulseIn(19, HIGH);
  float distance = (duration * 0.0343) / 2.0;

  Serial.print("dist: ");
  Serial.println(distance);
  return distance;
}

void goAround(int dia, float distAway) { //dia = object diameter
  uint16_t r, g, b, cl;
  float wbase = 25.5;
  float ID = dia + (distAway); //inner diameter
  float OD = ID + (2 * wbase); //outer diameter
  int OS = 150; //outer speed
  int IS = (ID*OS) / OD; //inner speed

  tcaselect(color);
  apds.getColorData(&r, &g, &b, &cl);
  Serial.print("color");
  Serial.println(cl);
  while (cl > 70) {
    FLmotor.speed(OS);
    BLmotor.speed(OS);
    FRmotor.speed(IS);
    BRmotor.speed(IS);
    apds.getColorData(&r, &g, &b, &cl);
  }
}

/*
bool detectBall() {
  tcaselect(bottom);
  int btof = tof.readRangeContinuousMillimeters();
  Serial.print("btof: ");
  Serial.println(btof);
  tcaselect(top);
  int ttof = tof.readRangeContinuousMillimeters();
  tcaselect(bottom);
  Serial.print("ttof: ");
  Serial.println(ttof);

  if (ttof - btof > 55) {
    Serial.print("diff: ");
    Serial.println((ttof - btof));
    return true;
  }
  return false;

}

void straightLineObst() {
  tcaselect(side);
  Serial.println("enter straightLineObst------------------------");
  //turn right until side tof sees object
  while (tof.readRangeContinuousMillimeters() > 200) {
    FRmotor.speed(128);
    BRmotor.speed(128);
    FLmotor.speed(-128);
    BLmotor.speed(-128);
  }

  //go forward until object is no longer visible
  while (tof.readRangeContinuousMillimeters() < 200) {
    speedSet(75);
  }

  //go forward a little bit more
  straight(625);

  left(90);

  while (tof.readRangeContinuousMillimeters() > 200) {
    speedSet(75);
  }

  //go forward until object is no longer visible
  while (tof.readRangeContinuousMillimeters() < 200) {
    speedSet(75);
  }

  //go forward a little bit more
  straight(625);

  left(90);

  while (tof.readRangeContinuousMillimeters() > 200) {
    speedSet(75);
  }

  while (Serial1.available()) {
    Serial1.read();
  }

}

void obstacleDetection() {
  tcaselect(top);
  if (tof.readRangeContinuousMillimeters() < 130) {
    straightLineObst();
  }
}
*/
