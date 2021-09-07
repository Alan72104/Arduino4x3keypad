#include <Arduino.h>
#include <FastLED.h>
#include "Effect.h"
#include "KeypadParams.h"
#include "main.h"
#include "RangedFloat.h"

class SpinningRainbow : public Effect
{
private:
    float GetPointToCenterDegrees(float x, float y)
    {
        // It works and it works
        float angle = atan2(y - HEIGHT / 2.0f, x - WIDTH / 2.0f) * 180 / PI + 90;
	    return (angle < 0) ? angle + 360 : angle;
    }

    RangedFloat* rainbowState;
    uint8_t pointToCenterAngles[HEIGHT][WIDTH]; // Angles mapped from 0-360 to 0-255

public:
    void Load() override
    {
        rainbowState = new RangedFloat(255.0f);
        for (uint8_t y = 0; y < HEIGHT; y++)
            for (uint8_t x = 0; x < WIDTH; x++)
                pointToCenterAngles[y][x] = map(GetPointToCenterDegrees(x + 0.5f, y + 0.5f), 0, 360, 0, 255);
    }

    void Unload() override
    {
        delete rainbowState;
    }

    void Update(float secondsElapsed) override
    {
        (*rainbowState) -= secondsElapsed * 30;
        
        rgb.Clear();

        for (uint8_t y = 0; y < HEIGHT; y++)
            for (uint8_t x = 0; x < WIDTH; x++)
                rgb.DrawPixel2d(x, y, CHSV(pointToCenterAngles[y][x] + rainbowState->Int(), 255, rgb.GetBrightness()));
    }
};