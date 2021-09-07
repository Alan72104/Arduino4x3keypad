#include <FastLED.h>
#include "Effect.h"
#include "main.h"

class FractionalDrawingTest2d : public Effect
{
private:
    float y;
    float x;

public:
    void Load() override
    {
        x = 0.0f;
        y = 0.0f;
    }

    void Update(float secondsElapsed) override
    {
        rgb.Clear();
        rgb.DrawSquare2d(x, y, 1.1, CHSV(128, 255, rgb.GetBrightness()) );
    }

    void HandleKey(uint8_t currentState, uint8_t keyX, uint8_t keyY)
    {
        if (currentState == HIGH)
        {
            if (keyY == 0 && keyX == 0 && y > 0.0f)        y -= 0.1f;
            else if (keyY == 1 && keyX == 0 && y < HEIGHT) y += 0.1f;
            else if (keyY == 1 && keyX == 2 && x < WIDTH)  x += 0.1f;
            else if (keyY == 1 && keyX == 1 && x > 0.0f)   x -= 0.1f;
        }
    }
};