#include <FastLED.h>
#include <vector>
#include "KeypadParams.h"
#include "main.h"
#include "Effect.h"

class ShootingParticles : public Effect
{
private:
    typedef struct
    {
        float x;
        float y;
        float vX;
        float vY;
        CRGB color;
    } Particle;

    Particle MakeParticle(float x, float y, float vX, float vY, CRGB color)
    {
        Particle newParticle;
        newParticle.x = x;
        newParticle.y = y;
        newParticle.vX = vX;
        newParticle.vY = vY;
        newParticle.color = color;
        return newParticle;
    }

    std::vector<Particle> particles;

public:
    void Load() override
    {
        std::vector<Particle>().swap(particles);
    }

    void Update(float secondsElapsed) override
    {
        FastLED.clear();

        for (auto particle = particles.begin(); particle != particles.end();)
        {
            particle->x += particle->vX * secondsElapsed;
            particle->y += particle->vY * secondsElapsed;

            rgb.DrawSquare2d(particle->x, particle->y, 1.0f, particle->color);

            if (particle->x <= -1.0f || particle->x >= WIDTH + 1.0f || particle->y <= -1.0f || particle->y >= HEIGHT + 1.0f)
                particles.erase(particle);
            else
                particle++;
        }
    }

    void HandleKey(uint8_t currentState, uint8_t keyX, uint8_t keyY) override
    {
        if (currentState == HIGH && particles.size() < 16)
        {
            float vX = WIDTH / 2 - (keyX + 0.5f);
            float vY = HEIGHT / 2 - (keyY + 0.5f);
            float l = sqrt(vX * vX + vY * vY);
            // The length of the vector we got by subtracting the key coordinate from the center is proportional to (or scaled by) the distance between the key and the center,
            // first normalize the vector to equalize the different "speeds" we could get, because we only want the "direction" here, after then we could scale it back by a constant we want
            particles.push_back(MakeParticle(keyX, keyY, (vX / l) * 8.0f, (vY / l) * 8.0f, CHSV(random(256), 255, rgb.GetBrightness())));
        }
    }

    std::string GetName() override { return "shootingParticles"; }
};