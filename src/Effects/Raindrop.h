#include <Arduino.h>
#include <FastLED.h>
#include "KeypadParams.h"
#include "main.h"
#include "Effect.h"

class Raindrop : public Effect
{
private:
    typedef struct {
        uint8_t x;
        float y;
        CRGB color;
    } Drop;

    Drop MakeRaindrop(uint8_t x, float y, CRGB color)
    {
        Drop newDrop;
        newDrop.x = x;
        newDrop.y = y;
        newDrop.color = color;
        return newDrop;
    }

    std::vector<Drop> drops;
    float delayElapsed;

public:
    void Load() override
    {
        delayElapsed = 0.0f;
    }

    void Update(float secondsElapsed) override
    {
        delayElapsed += secondsElapsed;

        if (delayElapsed >= 0.3f)
        {
            delayElapsed = 0.0f;
            drops.push_back(MakeRaindrop(Random(WIDTH), 0, CRGB(CHSV(random(256), 255, rgb.GetBrightness()))) );
        }

        rgb.Clear();

        for (auto drop = drops.begin(); drop != drops.end();)
        {
            drop->y += 4.0f * secondsElapsed;

            rgb.DrawSquare2d(drop->x, drop->y, 1.0f, drop->color);

            if (drop->y >= HEIGHT)
                drops.erase(drop);
            else
                drop++;
        }
    }

    std::string GetName() override { return "raindrop"; }
};