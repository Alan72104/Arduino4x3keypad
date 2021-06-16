#include <FastLED.h>
#include <algorithm>
#include <numeric>
#include "KeypadParams.h"
#include "main.h"
#include "Effect.h"

class BullsNCows : public Effect
{
private:
    enum GameState
    {
        ready,
        playing,
        score
    };

    float delayElapsed;
    GameState state;
    uint8_t startupState;
    uint8_t startupStep;
    bool isGuessing;
    uint8_t secretNum[4];
    uint8_t guessingNum[4];
    uint8_t numA;
    uint8_t numB;
    const uint8_t numCount = 6;
    bool win;
    float winElapsed;

public:
    void Load() override
    {
        delayElapsed = 0.0f;
        state = ready;
        startupState = 0;
        startupStep = 0;
    }

    void Update(float secondsElapsed) override
    {
        switch (state)
        {
            case ready:
                delayElapsed += secondsElapsed;

                if (delayElapsed >= 0.1f)
                {
                    delayElapsed = 0.0f;
                    startupStep++;
                    if (startupStep >= 4)
                    {
                        startupStep = 0;
                        startupState++;
                    }
                }

                if (startupState > 2)
                {
                    delayElapsed = 0.0f;
                    state = playing;
                    uint8_t ta[numCount];
                    uint8_t t = 0;
                    for (uint8_t i = 0; i < numCount; i++)
                        ta[i] = t++;
                    std::random_shuffle(ta, ta + numCount);
                    std::copy(ta, ta + 4, secretNum);
                    std::fill_n(guessingNum, 4, 0);
                    isGuessing = true;
                    win = false;
                    // Fall through...
                }
                else
                {
                    rgb.Clear();

                    for (uint8_t i = 0; i < startupStep + 1; i++)
                        switch (startupState)
                        {
                            case 0:
                                rgb.Draw(4 * 1 + i, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                                break;
                            case 1:
                                rgb.Draw(4 * 1 + i, CHSV(HUE_GREEN, 255, rgb.GetBrightness()));
                                break;
                            case 2:
                                rgb.Draw(4 * 1 + i, CHSV(HUE_BLUE, 255, rgb.GetBrightness()));
                                break;
                        }
                    break;
                }

            case playing:
                if (delayElapsed >= 5.0f)
                {
                    delayElapsed = 0.0f;
                    state = score;
                    winElapsed = 0.0f;
                    // Fall through...
                }
                else
                {
                    rgb.Clear();

                    if (isGuessing)
                    {
                        for (uint8_t i = 0; i < 4; i++)
                            rgb.Draw(4 * 1 + i, CHSV(rainbowHues[guessingNum[i]], 255, rgb.GetBrightness()));
                        rgb.Draw(4 * 2 + 3, CHSV(HUE_PINK, 255, rgb.GetBrightness()));
                    }
                    else
                    {
                        switch (numA)
                        {
                            case 4:
                                rgb.Draw(4 * 1 + 1, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                            case 3:
                                rgb.Draw(4 * 2 + 0, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                            case 2:
                                rgb.Draw(4 * 1 + 0, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                            case 1:
                                rgb.Draw(4 * 0 + 0, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                        }
                        switch (numB)
                        {
                            case 4:
                                rgb.Draw(4 * 1 + 2, CHSV(HUE_GREEN, 255, rgb.GetBrightness()));
                            case 3:
                                rgb.Draw(4 * 2 + 3, CHSV(HUE_GREEN, 255, rgb.GetBrightness()));
                            case 2:
                                rgb.Draw(4 * 1 + 3, CHSV(HUE_GREEN, 255, rgb.GetBrightness()));
                            case 1:
                                rgb.Draw(4 * 0 + 3, CHSV(HUE_GREEN, 255, rgb.GetBrightness()));
                        }
                    }

                    if (win)
                        delayElapsed += secondsElapsed;
                    break;
                }

            case score:
                delayElapsed += secondsElapsed;
                if (delayElapsed >= 4.0f)
                {
                    delayElapsed = 0.0f;
                    state = ready;
                    startupState = 0;
                    startupStep = 0;
                    break;
                }

                winElapsed += secondsElapsed;
                if (winElapsed >= 0.4f)
                {
                    winElapsed = 0.0f;
                    bool lastIsEmpty = !rgb.GetColor(0);
                    rgb.Fill(CHSV(HUE_YELLOW, 255, lastIsEmpty ? rgb.GetBrightness() : 0));
                }
                break;
        }
    }

    void HandleKey(uint8_t currentState, uint8_t keyX, uint8_t keyY) override
    {
        if (currentState != HIGH) return;

        if (isGuessing)
        {
            if (keyY == 1)
            {
                guessingNum[keyX]++;
                if (guessingNum[keyX] >= 4)
                    guessingNum[keyX] = 0;
            }
            else if (keyY == 2 && keyX == 3)
            {
                isGuessing = false;
                CalculateScore();
            }
        }
        else
            if (!win)
                isGuessing = true;
    }

    std::string GetName() override { return "bullsNCows"; }
    bool IsGameEffect() override { return true; }

private:
    void CalculateScore()
    {
        uint8_t bulls = 0;
        int8_t bucket[numCount] = {};

        for (uint8_t i = 0; i < 4; i++)
        {
            if (secretNum[i] == guessingNum[i])
                bulls++;
            else
            {
                bucket[secretNum[i]]++; 
                bucket[guessingNum[i]]--;
            }
        }
        numA = bulls;
        numB = 4 - bulls - std::accumulate(bucket, bucket + numCount, 0, [](int cur, int n) { if (n > 0) return cur + n; else return cur; });
        if (numA == 4 && numB == 0)
            win = true;
    }
};