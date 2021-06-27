#include <FastLED.h>
#include <string>
#include "Effect.h"
#include "main.h"

class Ripple : public Effect
{
private:
    typedef struct {
        uint8_t x;
        uint8_t y;
        float radius;
        CRGB color;
    } Circle;

    Circle MakeCircle(uint8_t x, uint8_t y, float radius, CRGB color)
    {
        Circle newCircle;
        newCircle.x = x;
        newCircle.y = y;
        newCircle.radius = radius;
        newCircle.color = color;
        return newCircle;
    }

    std::vector<Circle> circles;

public:
    void Load() override {}
    
    void Update(float secondsElapsed) override
    {
        rgb.Clear();

        for (auto circle = circles.begin(); circle != circles.end();)
        {
            circle->radius += 15.0f * secondsElapsed;

            rgb.DrawCircle2d(circle->x, circle->y, circle->radius, circle->color);

            if (circle->radius >= 5.0f)
                circles.erase(circle);
            else
                circle++;
        }
    }

    void OnKeyPressed(uint8_t state, uint8_t keyX, uint8_t keyY)
    {
        if (state == HIGH && circles.size() < 16)
            circles.push_back(MakeCircle(keyX, keyY, 0, CHSV(random(256), 255, rgb.GetBrightness())) );
    }

    std::string GetName() override { return "ripple"; }
};