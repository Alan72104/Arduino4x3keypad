#include <FastLED.h>
#include "Effect.h"
#include "KeypadParams.h"
#include "main.h"

class Stars : public Effect
{
private:
    float delayElapsed;

public:
    void Load() override
    {
        delayElapsed = 0.0f;
        for (uint8_t i = 0; i < NUM_LEDS; i++)
            rgb.Draw(i, CHSV(rainbowHues[random(7)], 255, rgb.GetBrightness()));
        // rgb.FillHSV([&]() { return rainbowHues[random(7)]; }, 255, rgb.GetBrightness());
    }

    void Update(float secondsElapsed) override
    {
        delayElapsed += secondsElapsed;

        if (delayElapsed >= 0.2f)
        {
            delayElapsed = 0.0f;
            rgb.Draw(Random(NUM_LEDS), CHSV(rainbowHues[random(7)], 255, rgb.GetBrightness()));
        }
    }
};