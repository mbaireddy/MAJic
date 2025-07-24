#pragma once

float hightarget(float target);
float lowtarget(float target);
void mstop(int seconds);
void right(int degree);
void left(int degree);
void rightgr(int turndegree);
void leftgr(int turndegree);
void doublegr();
void obstacle();
void straight(int seconds);

float ultrasonic();
void goAround(int dia, float distAway);
void straightLineObst();
void obstacleDetection();
bool detectBall();
void evacmove();
