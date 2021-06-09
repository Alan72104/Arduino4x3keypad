#ifndef MAIN_H
#define MAIN_H

#include <vector>
#include <memory>
#include "Keypad.h"
#include "Rgb.h"
#include "Effect.h"
#include "EffectManager.h"

// #define Debug

extern Keypad keypad;
extern Rgb rgb;
extern EffectManager effectManager;

void UpdateLed();
void CheckSerialMessage();

#endif