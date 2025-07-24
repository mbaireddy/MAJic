#include "common.h"
#include "turn.h"
#include "Motor.h"
#define TEXT_SIZE 1


int target = 0;
const int turn_range = 4;
int targetSpeed = 90;
//int turndegree = 55;
bool over = false;
bool under = false;


void mstop(int seconds) {
  speedSet(0);
  delay(seconds);
}

void right(int degree) {
  FRmotor.speed(128);
  BRmotor.speed(128);
  FLmotor.speed(-128);
  BLmotor.speed(-128);
  int delayTime = degree * 10;
  delay(delayTime);
}

void left(int degree) {
  FRmotor.speed(-128);
  BRmotor.speed(-128);
  FLmotor.speed(128);
  BLmotor.speed(128);
  int delayTime = degree * 10;
  delay(delayTime);
}

void straight(int seconds) {
  //speedSet(60);
  FRmotor.speed(60);
  BRmotor.speed(60);
  FLmotor.speed(60);
  BLmotor.speed(60);
  delay(seconds);
}



float hightarget(float target) {
  float high = 0;
  over = false;
  if (target + turn_range > 360){
    high = (target + turn_range) - 360;
    over = true;
  }
  else
    high = target + turn_range;
  return high;
}

float lowtarget(float target) {
  float low = 0;
  under = false;
  if (target - turn_range < 0){
    low = (target - turn_range) + 360;
    under = true;
  }
  else
    low = target - turn_range;
  return low;
}



void rightgr(int turndegree) {
  tcaselect(1);
  bno.getEvent(&event);
  Serial.print("START BNO: ");
  Serial.println(event.orientation.x);
  if (event.orientation.x + turndegree > 360) {
    target = event.orientation.x + turndegree - 360;
  } else {
    target = event.orientation.x + turndegree;
  }

  float h_target = hightarget(target);
  float l_target = lowtarget(target);
  //while (!(event.orientation.x < h_target && (event.orientation.x > l_target))) { //while its NOT in the range
  while((over&&event.orientation.x > h_target) || (under&&event.orientation.x < l_target) || (!over)&&(!under)&&(!(event.orientation.x < h_target && (event.orientation.x > l_target)))) {
    bno.getEvent(&event);
    Serial.print("BNO: ");
    Serial.println(event.orientation.x);
    BRmotor.speed(targetSpeed);
    FRmotor.speed(targetSpeed);
    BLmotor.speed(-(targetSpeed));
    FLmotor.speed(-(targetSpeed));
    //delay(50)
    
  }
  //mstop(); //i dont think it stops?
  //delay(4000);
  BRmotor.speed(0);
  FRmotor.speed(0);
  BLmotor.speed(0);
  FLmotor.speed(0);
  Serial.print("END BNO: ");
  Serial.println(event.orientation.x);
  //tone(22, 200, 200);

      while (Serial1.available()) {
        Serial1.read();
      }
  
}


void leftgr(int turndegree) {
  tcaselect(1);
  bno.getEvent(&event);
  Serial.print("START BNO: ");
  Serial.println(event.orientation.x);
  if (event.orientation.x - turndegree < 0) {
    target = event.orientation.x - turndegree + 360;
  } else {
    target = event.orientation.x - turndegree;
  }


  float h_target = hightarget(target);
  float l_target = lowtarget(target);
  //while (!(event.orientation.x < h_target && event.orientation.x > l_target)) { //while its NOT in the range
  while((over&&event.orientation.x > h_target) || (under&&event.orientation.x < l_target) || (!over)&&(!under)&&(!(event.orientation.x < h_target && (event.orientation.x > l_target)))) {  
    bno.getEvent(&event);
    Serial.print("BNO: ");
    Serial.println(event.orientation.x);
    BRmotor.speed(-(targetSpeed));
    FRmotor.speed(-(targetSpeed));
    BLmotor.speed(targetSpeed);
    FLmotor.speed(targetSpeed);
    //delay(50);
    
  }
  BRmotor.speed(0);
  FRmotor.speed(0);
  BLmotor.speed(0);
  FLmotor.speed(0);
  Serial.print("END BNO: ");
  Serial.println(event.orientation.x);
  //tone(22, 200, 200);
      while (Serial1.available()) {
        Serial1.read();
      }
  
}


void doublegr() {
  tcaselect(1);
  bno.getEvent(&event);
  Serial.print("START BNO: ");
  Serial.println(event.orientation.x);
  if (event.orientation.x + 180 > 360) {
    target = event.orientation.x + 180 - 360;
  } else
    target = event.orientation.x + 180;

  float h_target = hightarget(target);
  float l_target = lowtarget(target);

  //while (!(event.orientation.x < h_target && event.orientation.x > l_target)) { //while its NOT in the range
  while((over&&event.orientation.x > h_target) || (under&&event.orientation.x < l_target) || (!over)&&(!under)&&(!(event.orientation.x < h_target && (event.orientation.x > l_target)))) {
    bno.getEvent(&event);
    Serial.print("BNO: ");
    Serial.println(event.orientation.x);
    BRmotor.speed(-(targetSpeed));
    FRmotor.speed(-(targetSpeed));
    BLmotor.speed(targetSpeed);
    FLmotor.speed(targetSpeed);
    //delay(50);
  }
  BRmotor.speed(0);
  FRmotor.speed(0);
  BLmotor.speed(0);
  FLmotor.speed(0);
  Serial.print("END BNO: ");
  Serial.println(event.orientation.x);
  //tone(22, 200, 200);

        while (Serial1.available()) {
        Serial1.read();
      }
}
