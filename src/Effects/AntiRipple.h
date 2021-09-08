#include <FastLED.h>
#include "Effect.h"
#include "main.h"

class AntiRipple : public Ripple
{
private:

public:
    void Update(float secondsElapsed) override
    {
        rgb.Clear();

        for (auto circle = circles.begin(); circle != circles.end();)
        {
            circle->radius -= 15.0f * secondsElapsed;

            if (circle->radius > 0.0f)
                rgb.DrawCircle2d(circle->x, circle->y, circle->radius, circle->color);

            if (circle->radius <= 0.0f)
                circles.erase(circle);
            else
                circle++;
        }
    }

    void OnKeyPressed(uint8_t state, uint8_t keyX, uint8_t keyY) override
    {
        if (state == HIGH && circles.size() < 16)
            circles.push_back(MakeCircle(keyX, keyY, 5.0f, CHSV(random(256), 255, rgb.GetBrightness())) );
    }
};