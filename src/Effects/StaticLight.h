#include <FastLED.h>
#include <string>
#include "Effect.h"
#include "KeypadParams.h"
#include "main.h"

class StaticLight : public Effect
{
private:
    uint8_t state = 0;

public:
    void Load() override
    {
        state = 0;
    }

    void Update(float secondsElapsed) override
    {
        if (state == 7)
            rgb.Fill(CRGB(rgb.GetBrightness(), rgb.GetBrightness(), rgb.GetBrightness()));
        else
            rgb.Fill(CHSV(rainbowHues[state], 255, rgb.GetBrightness()));
    }

    void NextState()
    {
        if (++state >= 8) state = 0;
    }

    std::string GetName() override { return "staticLight"; }
};