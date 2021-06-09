#include <Arduino.h>
#include <FastLED.h>
#include "main.h"
#include "Effect.h"

class AntiRipple : public Effect
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
    void Load() override
    {
        std::vector<Circle>().swap(circles);
    }

    void Update(float secondsElapsed) override
    {
        FastLED.clear();

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

    void HandleKey(uint8_t currentState, uint8_t keyX, uint8_t keyY)
    {
        if (currentState == HIGH && circles.size() < 16)
            circles.push_back(MakeCircle(keyX, keyY, 0, CHSV(random(256), 255, rgb.GetBrightness())) );
    }

    std::string GetName() override { return "ripple"; }
};