#include <Arduino.h>
#include <FastLED.h>
#include <string>
#include "Effect.h"
#include "KeypadParams.h"
#include "main.h"

class SpinningRainbow : public Effect
{
private:
    float GetPointToCenterDegrees(float x, float y)
    {
        // It work and it works
        float angle = atan2(y - HEIGHT / 2.0f, x - WIDTH / 2.0f) * 180 / PI + 90;
	    return (angle < 0) ? angle + 360 : angle;
    }

    uint8_t rainbowState;
    uint8_t pointToCenterAngles[HEIGHT][WIDTH]; // Angles mapped from 0-360 to 0-255

public:
    void Load() override
    {
        rainbowState = 0;
        for (uint8_t y = 0; y < HEIGHT; y++)
            for (uint8_t x = 0; x < WIDTH; x++)
                pointToCenterAngles[y][x] = map(GetPointToCenterDegrees(x + 0.5f, y + 0.5f), 0, 360, 0, 255);
    }

    void Update(float secondsElapsed) override
    {
        rainbowState--;
        
        rgb.Clear();

        for (uint8_t y = 0; y < HEIGHT; y++)
            for (uint8_t x = 0; x < WIDTH; x++)
                rgb.DrawPixel2d(x, y, CHSV(pointToCenterAngles[y][x] + rainbowState, 255, rgb.GetBrightness()));
    }

    std::string GetName() override { return "spinningRainbow"; }
};