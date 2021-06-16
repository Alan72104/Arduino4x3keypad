#include <Arduino.h>
#include <string>
#include "KeypadParams.h"
#include "Rgb.h"
#include "main.h"
#include "Effect.h"

class StaticRainbow : public Effect
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

        for (uint8_t i = 0; i < HEIGHT; i++)
            for (uint8_t j = 0; j < WIDTH; j++)
                rgb.Draw(4 * i + j, keypad.GetState(j, i) ? CHSV((4 * i + j) * 25 - (rainbowState * 1), 255, rgb.GetBrightness())
                                                            : CHSV(0, 0, 0));
    }

    std::string GetName() override { return "staticRainbow"; }
};