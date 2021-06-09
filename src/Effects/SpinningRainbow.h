#include <Arduino.h>
#include <FastLED.h>
#include "main.h"
#include "Effect.h"

class SpinningRainbow : public Effect
{
private:
    uint8_t rainbowState;

public:
    void Load() override
    {
        rainbowState = 0;
    }

    void Update(float secondsElapsed) override
    {
        rainbowState++;
        
        rgb.Clear();

        rgb.DrawSquare2d(0.5f, 0.5f, 1, CHSV(rainbowState, 255, rgb.GetBrightness()));
        rgb.DrawSquare2d(2.5f, 0.5f, 1, CHSV(rainbowState + 64, 255, rgb.GetBrightness()));
        rgb.DrawSquare2d(0.5f, 1.5f, 1, CHSV(rainbowState + 64 * 2, 255, rgb.GetBrightness()));
        rgb.DrawSquare2d(2.5f, 1.5f, 1, CHSV(rainbowState + 64 * 3, 255, rgb.GetBrightness()));
    }

    std::string GetName() override { return "spinningRainbow"; }
};