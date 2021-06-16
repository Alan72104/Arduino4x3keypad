#include <Arduino.h>
#include <FastLED.h>
#include "KeypadParams.h"
#include "main.h"
#include "Effect.h"

class Breathing : public Effect
{
private:
    uint8_t breathingState;
    float delayElapsed;

public:
    void Load() override
    {
        breathingState = 0;
        delayElapsed = 0.0f;
    }

    void Update(float secondsElapsed) override
    {
        delayElapsed += secondsElapsed;

        if (delayElapsed >= 4.0f)
        {
            delayElapsed = 0.0f;
            breathingState += 1;
            if (breathingState > 6)
                breathingState = 0;
        }

        rgb.Fill(CHSV(rainbowHues[breathingState], 255, (max(127, rgb.GetBrightness()) *
                      (delayElapsed <= 4.0f / 2 ? delayElapsed / 2 : (2.0f - (delayElapsed - 2.0f)) / 2))) );
    }

    std::string GetName() override { return "breathing"; }
};