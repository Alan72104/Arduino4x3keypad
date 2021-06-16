#include <Arduino.h>
#include <FastLED.h>
#include "KeypadParams.h"
#include "main.h"
#include "Effect.h"

class StaticLight : public Effect
{
private:
    uint8_t staticLightState = 0;

public:
    void Load() override
    {
        staticLightState = 0;
    }

    void Update(float secondsElapsed) override
    {
        if (staticLightState == 7)
            rgb.Fill(CRGB(rgb.GetBrightness(), rgb.GetBrightness(), rgb.GetBrightness()));
        else
            rgb.Fill(CHSV(rainbowHues[staticLightState], 255, rgb.GetBrightness()));
    }

    void NextState()
    {
        if (++staticLightState >= 8) staticLightState = 0;
    }

    std::string GetName() override { return "staticLight"; }
};