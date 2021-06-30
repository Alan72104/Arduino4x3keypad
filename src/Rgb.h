#ifndef RGB_H
#define RGB_H

#include <FastLED.h>
#include <algorithm>
// #include <ArxTypeTraits.h>
#include "KeypadParams.h"

class Rgb
{
private:
    CRGB leds[NUM_LEDS];
    uint8_t brightness = 63;

    void DrawCircle2d_internal(uint8_t xc, uint8_t yc, uint8_t x, uint8_t y, CRGB color);
    // template<class T, std::enable_if<std::is_function<T>>>
    //     uint8_t FillHSV_getValue(T t) { return t(); }
    // template<class T, std::enable_if<not std::is_function<T>>>
    //     uint8_t FillHSV_getValue(T t) { return t; }

public:
    void Init();
    CRGB GetColor(uint8_t i);
    template<class Color>
        void Draw(uint8_t i, Color color) { leds[i] = color; }
    template<class Color>
        void Blend(uint8_t i, Color color) { leds[i] += color; }
    template<class Color>
        void Fill(Color color) { std::fill_n(leds, NUM_LEDS, color); }
    // template<class H, class S, class V>
    //     CRGB FillHSV(H h, S s, V v)
    //     {
    //         std::generate_n(leds, NUM_LEDS, [&]() { return CHSV(FillHSV_getValue(h), FillHSV_getValue(s), FillHSV_getValue(v)); });
    //         return leds[0];
    //     }
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