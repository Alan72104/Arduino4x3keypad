#include <Arduino.h>
#include <FastLED.h>
#include "KeypadParams.h"
#include "main.h"
#include "Effect.h"

class Rainbow : public Effect
{
private:
    uint8_t rainbowState;

public:
    void Load() override
    {
        rainbowState = 0;
    }

    void Update(float secondsElapsed) override
    {
        rainbowState++;

        for (uint8_t i = 0; i < WIDTH; i++)
            rgb.SetColor(4*0+i,
                rgb.SetColor(4*1+i,
                    rgb.SetColor(4*2+i, CHSV(i * 10 - (rainbowState * 1), 255, rgb.GetBrightness()) )));
    }

    std::string GetName() override { return "rainbow"; }
};