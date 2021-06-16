#include <FastLED.h>
#include "KeypadParams.h"
#include "main.h"
#include "Effect.h"

class WhacAMole : public Effect
{
private:
    enum GameState
    {
        ready,
        playing,
        score
    };

    float delayElapsed;
    GameState moleState;
    bool moleIsHere;
    uint8_t moleX;
    uint8_t moleY;
    uint8_t moleScore;
    float moleSpawningDelay;
    uint8_t moleSpawnCount;

public:
    void Load() override
    {
        moleState = ready;
        moleIsHere = false;
        delayElapsed = 0.0f;
    }

    void Update(float secondsElapsed) override
    {
        rgb.Clear();

        switch (moleState)
        {
            case ready:
                delayElapsed += secondsElapsed;

                if (delayElapsed >= 4.0f)
                {
                    delayElapsed = 0.0f;
                    moleState = playing;
                    moleSpawningDelay = 0.0f;
                    moleSpawnCount = 0;
                    moleScore = 0;
                    // Fall through...
                }
                else
                {
                    for (uint8_t i = 0; i < min((int)delayElapsed, 3) + 1; i++)
                        for (uint8_t j = 0; j < HEIGHT; j++)
                            rgb.Draw(j * WIDTH + i, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                    break;
                }

            case playing:
                if (moleSpawnCount >= 30)
                {
                    delayElapsed += secondsElapsed;
                    if (delayElapsed > 0.5f)
                    {
                        moleState = score;
                        moleIsHere = false;
                        // Fall through...
                    }
                    else
                        break;
                }
                else
                {
                    moleSpawningDelay += secondsElapsed;
                    if (moleSpawningDelay >= 0.5f)
                    {
                        moleSpawningDelay = 0.0f;
                        moleX = random(WIDTH);
                        moleY = random(HEIGHT);
                        moleIsHere = true;
                        moleSpawnCount++;
                    }
                    if (moleIsHere)
                        rgb.Draw(moleY * WIDTH + moleX, CHSV(random(256), 255, rgb.GetBrightness()));
                    break;
                }

            case score:
                delayElapsed += secondsElapsed;
                if (delayElapsed > 5.0f)
                {
                    delayElapsed = 0.0f;
                    moleState = ready;
                    moleIsHere = false;
                    break;
                }

                rgb.DrawLine(0.0f, moleScore * NUM_LEDS / 30.0f, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                break;
        }
    }

    void HandleKey(uint8_t currentState, uint8_t keyX, uint8_t keyY) override
    {
        if (currentState == HIGH && moleIsHere)
            if (keyY == moleY && keyX == moleX)
            {
                moleScore++;
                moleIsHere = false;
            }
    }

    std::string GetName() override { return "whacAMole"; }
    bool IsGameEffect() override { return true; }
};