#include <FastLED.h>
#include "Effect.h"
#include "KeypadParams.h"
#include "main.h"

class Breathing : public Effect
{
private:
    uint8_t state;
    float delayElapsed;

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

        rgb.Fill(CHSV(rainbowHues[state], 255, (max(127, rgb.GetBrightness()) *
                      (delayElapsed <= 4.0f / 2 ? delayElapsed / 2 : (2.0f - (delayElapsed - 2.0f)) / 2))) );
    }
};