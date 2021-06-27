#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include "KeypadParams.h"
#include "main.h"
#include "Effect.h"

class Fire : public Effect
{
private:
    typedef struct {
        float x;
        float y;
        float vX;
        float vY;
    } FireParticle;

    FireParticle MakeParticle(float x, float y, float vX, float vY)
    {
        FireParticle newPart;
        newPart.x = x;
        newPart.y = y;
        newPart.vX = vX;
        newPart.vY = vY;
        return newPart;
    }

    std::vector<FireParticle> particles;

public:
    void Load() override {}

    void Update(float secondsElapsed) override
    {
        rgb.Clear();

        if (particles.size() < 16)
            particles.push_back(MakeParticle(frandom(-2.0f, 4.2f),
                                             3.0f,
                                             frandom(-1.0f, 1.0f),
                                             -frandom(1.0f, 2.0f) ));
                                             
        for (auto part = particles.begin(); part != particles.end();)
        {
            part->x += secondsElapsed * part->vX;
            part->y += secondsElapsed * part->vY;

            rgb.DrawSquare2d(part->x, part->y, 0.2f, CRGB::Red);

            if (part->x <= -0.2f || part->y >= 4.2f)
                particles.erase(part);
            else
                part++;
        }
    }

    std::string GetName() override { return "fire"; }
};