#include <FastLED.h>
#include <string>
#include "Effect.h"
#include "main.h"

class FractionalDrawingTest2d : public Effect
{
private:
    float fractionalDrawingTestY;
    float fractionalDrawingTestX;

public:
    void Load() override
    {
        fractionalDrawingTestX = 0.0f;
        fractionalDrawingTestY = 0.0f;
    }

    void Update(float secondsElapsed) override
    {
        rgb.Clear();
        rgb.DrawSquare2d(fractionalDrawingTestX, fractionalDrawingTestY, 1.1, CHSV(128, 255, rgb.GetBrightness()) );
    }

    void HandleKey(uint8_t currentState, uint8_t keyX, uint8_t keyY)
    {
        if (currentState == HIGH)
        {
            if (keyY == 0 && keyX == 0 && fractionalDrawingTestY > 0.0f)        fractionalDrawingTestY -= 0.1f;
            else if (keyY == 1 && keyX == 0 && fractionalDrawingTestY < HEIGHT) fractionalDrawingTestY += 0.1f;
            else if (keyY == 1 && keyX == 2 && fractionalDrawingTestX < WIDTH)  fractionalDrawingTestX += 0.1f;
            else if (keyY == 1 && keyX == 1 && fractionalDrawingTestX > 0.0f)   fractionalDrawingTestX -= 0.1f;
        }
    }

    std::string GetName() override { return "fractionalDrawingTest2d"; }
};