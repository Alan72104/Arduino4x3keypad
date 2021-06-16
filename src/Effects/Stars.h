#include <FastLED.h>
#include "KeypadParams.h"
#include "main.h"
#include "Effect.h"

class Stars : public Effect
{
private:
    float delayElapsed;

public:
    void Load() override
    {
        delayElapsed = 0.0f;
        rgb.Fill(CHSV(rainbowHues[random(7)], 255, rgb.GetBrightness()));
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

    std::string GetName() override { return "stars"; }
};