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
            rgb.Draw(4*0+i,
                rgb.Draw(4*1+i,
                    rgb.Draw(4*2+i, CHSV(i * 10 - (rainbowState->Int() * 1), 255, rgb.GetBrightness()) )));
    }

    std::string GetName() override { return "rainbow"; }
};