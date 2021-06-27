#include <FastLED.h>
#include <string>
#include <vector>
#include "Effect.h"
#include "KeypadParams.h"
#include "main.h"

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

        if (particles.size() < 8)
            particles.push_back(MakeParticle(frandom(0.0f + 0.5f, WIDTH - 0.5f - 0.2f),
                                             HEIGHT,
                                             frandom(-1.0f, 1.0f) * 3.5f,
                                             -frandom(1.0f, 2.0f) * 2.5f ));
        
        for (auto part = particles.begin(); part != particles.end();)
        {
            part->x += secondsElapsed * part->vX;
            part->y += secondsElapsed * part->vY;

            rgb.DrawSquare2d(part->x, part->y, 0.2f, CHSV(HUE_RED, 255, constrain(rgb.GetBrightness(), 130, 255)));

            if (part->x <= -0.2f || part->x >= WIDTH + 0.2f || part->y <= -0.2f)
                particles.erase(part);
            else
                part++;
        }
    }

    std::string GetName() override { return "fire"; }
};