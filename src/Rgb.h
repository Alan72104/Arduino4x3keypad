#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include "KeypadParams.h"

class Rgb
{
    private:
        CRGB leds[NUM_LEDS];
        uint8_t ledBrightness = 63;
        
        void DrawCircle2d_internal(uint8_t xc, uint8_t yc, uint8_t x, uint8_t y, CRGB color);
    public:
        void Init();
        CRGB GetColorFraction(CRGB colorIn, float fraction);
        void DrawPixel2d(int x, int y, CRGB color);
        void DrawLine(float fPos, float length, CRGB color);
        void DrawSquare2d(float fX, float fY, float length, CRGB color);
        void DrawCircle2d(uint8_t xc, uint8_t yc, uint8_t r, CRGB color);
        void Draw();
};

#endif