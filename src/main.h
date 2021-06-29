#ifndef MAIN_H
#define MAIN_H

#include "EffectManager.h"
#include "Keypad.h"
#include "Rgb.h"

#undef min
#undef max
#undef constrain
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

// #define Debug

extern Keypad keypad;
extern Rgb rgb;
extern EffectManager effectManager;

uint32_t GetLoopTime();
void UpdateLed();
void CheckSerialMessage();
uint16_t Random(uint16_t max);
float frandom(float min, float max);

#endif