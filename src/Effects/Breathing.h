#include <FastLED.h>
#include "Effect.h"
#include "KeypadParams.h"
#include "main.h"

class Breathing : public Effect
{
private:
    uint8_t state;
    float delayElapsed;

    float Map(float aMin, float aMax, float bMin, float bMax, float v)
    {
        return (v - aMin) / (aMax - aMin) * (bMax - bMin) + bMin;
    }

public:
    void Load() override
    {
        state = 0;
        delayElapsed = 0.0f;
    }

    void Update(float secondsElapsed) override
    {
        delayElapsed += secondsElapsed;

        if (delayElapsed >= 4.0f)
        {
            delayElapsed = 0.0f;
            state += 1;
            if (state > 6)
                state = 0;
        }

        if (delayElapsed <= 2.0f)
            rgb.Fill(CHSV(rainbowHues[state], 255, Map(0.0f, 2.0f, 31.0f, max(127, rgb.GetBrightness()), delayElapsed)));
        else
            rgb.Fill(CHSV(rainbowHues[state], 255, Map(0.0f, 2.0f, max(127, rgb.GetBrightness()), 31.0f, delayElapsed - 2.0f)));
    }
};