#include <FastLED.h>
#include <string>
#include "Effect.h"
#include "KeypadParams.h"
#include "main.h"
#include "RangedFloat.h"

class Rainbow : public Effect
{
private:
    RangedFloat* rainbowState;

public:
    void Load() override
    {
        rainbowState = new RangedFloat(255.0f);
    }

    void Unload() override
    {
        delete rainbowState;
    }

    void Update(float secondsElapsed) override
    {
        (*rainbowState) += secondsElapsed * 30;

        for (uint8_t i = 0; i < WIDTH; i++)
            for (uint8_t j = 0; j < HEIGHT; j++)
                rgb.Draw(4*j+i, CHSV(i * 10 - (rainbowState->Int() * 1), 255, rgb.GetBrightness()));
    }

    std::string GetName() override { return "rainbow"; }
};