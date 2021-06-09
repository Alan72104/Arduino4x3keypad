#ifndef MAIN_H
#define MAIN_H

#include <vector>
#include <memory>
#include "Keypad.h"
#include "Rgb.h"
#include "Effect.h"
#include "EffectManager.h"

#undef min()
#undef max()
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

// #define Debug

extern Keypad keypad;
extern Rgb rgb;
extern EffectManager effectManager;

void UpdateLed();
void CheckSerialMessage();
uint16_t Random(uint16_t max);

#endif