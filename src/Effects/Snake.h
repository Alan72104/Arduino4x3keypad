#include <FastLED.h>
#include <deque>
#include <utility>
#include "Effect.h"
#include "KeypadParams.h"
#include "main.h"

class Snake : public Effect
{
private:
    float delayElapsed;
    uint8_t snakeHue;
    uint8_t snakeX;
    uint8_t snakeY;
    std::deque<std::pair<uint8_t, uint8_t>> snakePaths;

public:
    void Load() override
    {
        snakeX = random(WIDTH);
        snakeY = random(HEIGHT);
        snakeHue = random(256);
        delayElapsed = 0.0f;
    }
    
    void Update(float secondsElapsed) override
    {
        delayElapsed += secondsElapsed;

        if (delayElapsed >= 0.1f)
        {
            delayElapsed = 0.0f;

            snakePaths.push_back(std::make_pair(snakeX, snakeY));
            if (snakePaths.size() >= 4)
                snakePaths.pop_front();

            if (random(2))
            {
                if (snakeX == 0)              snakeX += 1;
                else if (snakeX == WIDTH - 1) snakeX -= 1;
                else                          snakeX += random(2) ? -1 : 1;
            }
            else
            {
                if (snakeY == 0)               snakeY += 1;
                else if (snakeY == HEIGHT - 1) snakeY -= 1;
                else                           snakeY += random(2) ? -1 : 1;
            }
        }

        rgb.Clear();

        for (uint8_t i = 0; i < snakePaths.size(); i++)
            rgb.DrawPixel2d(snakePaths[i].first, snakePaths[i].second, CHSV(snakeHue - 31, 255, (rgb.GetBrightness() / snakePaths.size()) * (i + 1)) );

        rgb.DrawPixel2d(snakeX, snakeY, CHSV(snakeHue, 255, rgb.GetBrightness()) );
    }
};