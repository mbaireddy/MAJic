#pragma once
#include <Arduino.h>

struct Motor
{
  uint8_t fpin;
  uint8_t rpin;
  void speed(int val);
} ;


extern Motor BRmotor, FRmotor, BLmotor, FLmotor;
