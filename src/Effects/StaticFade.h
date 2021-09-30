#include <FastLED.h>
#include "Effect.h"
#include "main.h"
#include "KeypadParams.h"

class StaticFade : public Effect
{
private:
    bool fading[NUM_LEDS] = {0};
    static constexpr float fadingRatio = 0.5f;

public:
    void Load() override
    {
        rgb.Clear();
    }
    
    void Update(float secondsElapsed) override
    {
        for (uint8_t i = 0; i < NUM_LEDS; i++)
        {
            if (fading[i])
            {
                CRGB color = rgb.GetColor(i);\
                rgb.Draw(i, color.fadeToBlackBy(255 * fadingRatio * secondsElapsed));
                if (color == CRGB(0))
                    fading[i] = false;
            }
        }
    }

    void OnKeyPressed(uint8_t state, uint8_t keyX, uint8_t keyY) override
    {
        if (state == HIGH)
        {
            fading[keyY * WIDTH + keyX] = true;
            rgb.Draw(keyY * WIDTH + keyX, CHSV(rainbowHues[Random(7)], 255, rgb.GetBrightness()));
        }
    }
};