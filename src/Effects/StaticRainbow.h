#include "Effect.h"
#include "KeypadParams.h"
#include "main.h"
#include "Rgb.h"
#include "RangedFloat.h"

class StaticRainbow : public Effect
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

        for (uint8_t i = 0; i < HEIGHT; i++)
            for (uint8_t j = 0; j < WIDTH; j++)
                rgb.Draw(4 * i + j, keypad.GetState(j, i) ? CHSV((4 * i + j) * 25 - (rainbowState->Int() * 1), 255, rgb.GetBrightness())
                                                            : CHSV(0, 0, 0));
    }
};