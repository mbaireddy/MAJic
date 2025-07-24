//ISSUE ON 6/8 WITH BNO: OVERTURNING EVEN THOUGH BNO VALUES CORRECT
//SOLUTION: HAVE TO SET MOTOR VALUES TO 0 AFTER THE WHILE LOOP

//ANOTHER ISSUE WAS GETTING LINKER ERROR
//SOLUTION: HAVE TO DECLARE THE EXTERNED VARIABLE AS GLOBAL
#include "common.h"
#include "turn.h"
#include "Motor.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_APDS9960.h"
#include <Adafruit_BNO055.h>
#include <VL53L0X.h>
#include <string>
#include <time.h>
#include <stdlib.h>

//#define ROOM1
#define ROOM2
//#define ROOM3

VL53L0X tof;
Adafruit_APDS9960 apds;
Adafruit_BNO055 bno;
sensors_event_t event;

Servo camservo;
Servo evacservo;
//TRIG is 26
//ECHO is 19

bool evacroom = false;
bool blackball = false;
int ball = 2;  //0 is the first silver ball, 1 is the second silver ball, 2 means move on to black ball
uint16_t r, g, b, cl;



void setup() {
  Serial1.setRX(17);
  Serial1.setTX(16);
  Serial1.begin(9600); //115200

  Serial.begin(115200);
  Wire.setSDA(28);
  Wire.setSCL(29);
  Wire.begin();
  delay(2000);

  pinMode(20, INPUT);

  Serial.println("Press GP20 to begin...");
  while (digitalRead(20)) delay(10);
  Serial1.write("123456");
  delay(1000);

  pinMode(26, OUTPUT); //trig for ping
  pinMode (19, INPUT); //echo for ping

  camservo.attach(0, 800, 2000);
  camservo.write(116);
  delay(1000);

  evacservo.attach(7, 600, 2300);
  evacservo.write(90);
  //levacservo.attach(2, 600, 2400);
  //levacservo.write(40);
  delay(2000);

  tcaselect(color);
  //Serial.println("test");
  if (!apds.begin()) {
    Serial.println("APDS allocation failed");
    for (;;)
      ;
  }
  Serial.println("test1");
  apds.enableColor(true);
  Serial.println("APDS allocation succeeded");


  tcaselect(1);

  if (!bno.begin()) {
    Serial.println("No BNO055 detected");
    while (1) delay(10);
  } else Serial.println("BNO055 found!");
  bno.setExtCrystalUse(true);
  //bno.getEvent(&event);


  /*tof.setTimeout(500);
    if (!tof.init()) {
    Serial.println("tof failed");
    while (1)
      ;
    }
    tof.startContinuous();
    Serial.println("tof initialized");*/
}

void loop() {
#ifdef ROOM1
  {

    tcaselect(color);
    apds.getColorData(&r, &g, &b, &cl);
    Serial.print("Clear: ");
    Serial.println(cl);

    while (cl < 650)  {
      int fleft = Serial1.parseInt();
      int fright = Serial1.parseInt();
      int turn = Serial1.parseInt();
      int bleft = fleft;
      int bright = fright;

      Serial.print("Turn: ");
      Serial.println(turn);

      //obstacleDetection();
      float dist = ultrasonic();
      if (dist < 12) {
        right(100);
        goAround(10, dist / 2);
      }

      else if (turn == 4) {
        //tone(22, 600, 500);
        
        mstop(500);
        //straight(500);
        //straight(500);
        continue;
      }
      else if (turn == 1) {
        straight(1600);
        rightgr(55);
      }
      else if (turn == 2) {
        straight(1600);
        leftgr(55);
      }
      else if (turn == 3) {
        straight(1700);
        doublegr();
      }
      else {
        FLmotor.speed(fleft);
        BLmotor.speed(bleft);
        FRmotor.speed(fright);
        BRmotor.speed(bright);
      }

      tcaselect(color);
      apds.getColorData(&r, &g, &b, &cl);
      Serial.print("Clear: ");
      Serial.println(cl);

      while (Serial1.available()) {
        Serial1.read();
      }
    }
  }
}
#endif

#ifdef ROOM2 //evac--------------------------------------------------------------------------------------------------------------
  
  Serial.println("EVACCCCCC");
  
  tone(22, 600, 1000);
  mstop(1000);
  Serial1.write("424242");
  evacroom = true;
  //camservo.write(70); //up
  //delay(1000);
  camservo.write(75);
  delay(1000);
  // straight(1200);
  
  /*tcaselect(color);
  apds.getColorData(&r, &g, &b, &cl);
  Serial.print("Clear: ");
  Serial.println(cl);*/
  int fleft = Serial1.parseInt();
  int fright = Serial1.parseInt();
  int result = Serial1.parseInt();
  int bleft = fleft;
  int bright = fright;

  Serial.print("fleft: ");
      Serial.print(fleft);
      Serial.print("     fright: ");
      Serial.print(fright);

  //RESULT KEY:
  /*
   * 0 = nothing
   * 1 = pickup
   * 2 = dropoff
   * 3 = black line
   * anything above 3 is a turn angle
   */
  Serial.print("    Result: ");
  Serial.println(result);
  while (result != 3)  {
    while (Serial1.available()) {
        Serial1.read();
      }
    
    if (result == 1) { // pickup
      //servo down
      //servo to default pos
      speedSet(0);
      evacservo.write(0);
      delay(500);
      evacservo.write(90);
      delay(500);
    }
    else if (result == 2) {
      //mstop(1000);
      speedSet(0);
      leftgr(160);
      speedSet(0);
      speedSet(-50);
      delay(400);
      evacservo.write(180);
      delay(500);
      evacservo.write(90);
      //raise servo
      //back to default pos
      straight(400);
    }/*
    else if(result > 3){
      rightgr(result);
    }
    else if(result < -3){
      leftgr(-result);
    }*/
    else if (result == 0) {
      FLmotor.speed(fleft);
      BLmotor.speed(bleft);
      FRmotor.speed(fright);
      BRmotor.speed(bright);
      
    }
  
    fleft = Serial1.parseInt();
    fright = Serial1.parseInt();
    result = Serial1.parseInt();
    bleft = fleft;
    bright = fright;

    Serial.print("fleft: ");
    Serial.print(fleft);
    Serial.print("     fright: ");
    Serial.print(fright);

    Serial.print("    Result: ");
    Serial.println(result);
  }


}





#endif

#ifdef ROOM3
{

  straight(1200);
  right(90);
  straight(500);



  //  !! ROOM 3 -------------------------------------------------------------------------------------------------------------------------------------

  Serial1.write("4242");
  evacroom = false;

  char c;
  tcaselect(color);
  apds.getColorData(&r, &g, &b, &cl);
  Serial.print("Clear: ");
  Serial.println(cl);

  while (r - g < 10)  { //RED COLOR
    int fleft = Serial1.parseInt();
    int fright = Serial1.parseInt();
    int turn = Serial1.parseInt();
    int bleft = fleft;
    int bright = fright;

    Serial.print("Turn: ");
    Serial.println(turn);

    //obstacleDetection();
    float dist = ultrasonic();
    if (dist < 12) {
      right(100);
      goAround(10, dist / 2);
    }

    else if (turn == 4) {
      //tone(22, 600, 500);
      mstop(500);
      //straight(500);
      continue;
    }
    else if (turn == 1) {
      straight(1300);
      rightgr(55);
    }
    else if (turn == 2) {
      straight(1300);
      leftgr(55);
    }
    else if (turn == 3) {
      straight(1700);
      doublegr();
    }
    else {
      FLmotor.speed(fleft);
      BLmotor.speed(bleft);
      FRmotor.speed(fright);
      BRmotor.speed(bright);
    }

    tcaselect(color);
    apds.getColorData(&r, &g, &b, &cl);
    Serial.print("red-green: ");
    Serial.println(r - g);

    while (Serial1.available())
    {
      Serial1.read();
    }
  }

  tone(22, 600, 1000);
  mstop(100000);
}

}

#endif
