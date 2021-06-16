#include <Arduino.h>
#include <FastLED.h>
#include "main.h"
#include "Effect.h"

class Splash : public Effect
{
private:
    typedef struct {
        float x;
        uint8_t y;
        int8_t direction;
        CRGB color;
    } Ball;

    Ball MakeBall(float x, uint8_t y, uint8_t direction, CRGB color)
    {
        Ball newBall;
        newBall.x = x;
        newBall.y = y;
        newBall.direction = direction;
        newBall.color = color;
        return newBall;
    }

    std::vector<Ball> balls;
    
public:
    void Load() override
    {
        std::vector<Ball>().swap(balls);
    }

    void Update(float secondsElapsed) override
    {
        rgb.Clear();

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

    void HandleKey(uint8_t currentState, uint8_t keyX, uint8_t keyY) override
    {
        if (currentState == HIGH && balls.size() < 16)
        {
            CRGB color = CHSV(random(256), 255, rgb.GetBrightness());
            balls.push_back(MakeBall(keyX, keyY, -1, color));
            balls.push_back(MakeBall(keyX, keyY, 1, color));
        }
    }

    std::string GetName() override { return "splash"; }
};