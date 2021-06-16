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
        for (uint8_t i = 0; i < NUM_LEDS; i++)
        {
            if (staticLightState == 7)
                rgb.Draw(i, CRGB(rgb.GetBrightness(), rgb.GetBrightness(), rgb.GetBrightness()));
            else
                rgb.Draw(i, CHSV(rainbowHues[staticLightState], 255, rgb.GetBrightness()));
        }
    }

    void NextState()
    {
        if (++staticLightState >= 8) staticLightState = 0;
    }

    std::string GetName() override { return "staticLight"; }
};