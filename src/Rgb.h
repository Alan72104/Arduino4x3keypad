#ifndef RGB_H
#define RGB_H

#include <Arduino.h>
#include <FastLED.h>
#include "KeypadParams.h"

class Rgb
{
private:
    CRGB leds[NUM_LEDS];
    uint8_t brightness = 63;

    void DrawCircle2d_internal(uint8_t xc, uint8_t yc, uint8_t x, uint8_t y, CRGB color);

public:
    void Init();
    CRGB GetColor(uint8_t i);
    CRGB Draw(uint8_t i, CRGB color);
    CRGB Draw(uint8_t i, CHSV color);
    CRGB Blend(uint8_t i, CRGB color);
    CRGB Blend(uint8_t i, CHSV color);
    CRGB Fill(CRGB color);
    CRGB Fill(CHSV color);
    uint8_t GetBrightness();
    void IncreaseBrightness();
    void DecreaseBrightness();
    CRGB GetColorFraction(CRGB colorIn, float fraction);
    void DrawPixel2d(int x, int y, CRGB color);
    void DrawLine(float fPos, float length, CRGB color);
    void DrawSquare2d(float fX, float fY, float length, CRGB color);
    void DrawCircle2d(uint8_t xc, uint8_t yc, uint8_t r, CRGB color);
    void Show();
    void Clear();
};

#endif