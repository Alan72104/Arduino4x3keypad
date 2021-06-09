#include <Arduino.h>
#include <FastLED.h>
#include "main.h"
#include "Effect.h"

class SpreadOut : public Effect
{
private:
    typedef struct {
        float x;
        uint8_t y;
        int8_t direction;
        CRGB color;
    } Ball;

    std::vector<Ball> balls;

    Ball MakeBall(float x, uint8_t y, uint8_t direction, CRGB color)
    {
        Ball newBall;
        newBall.x = x;
        newBall.y = y;
        newBall.direction = direction;
        newBall.color = color;
        return newBall;
    }
public:
    void Load() override
    {
        std::vector<Ball>().swap(balls);
        FastLED.clear();
    }

    void Update(float secondsElapsed) override
    {
        FastLED.clear();

        for (auto ball = balls.begin(); ball != balls.end();)
        {
            ball->x += ball->direction * 10.0f * secondsElapsed;

            rgb.DrawLine(4 * ball->y + constrain(ball->x, 0.5f, 3.5f) - 0.5f, 1, ball->color);

            if (ball->x < 0.0f || ball->x >= 4.0f)
                balls.erase(ball);
            else
                ball++;
        }
    }

    std::string GetName() override { return "spreadOut"; }
};