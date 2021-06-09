#include <FastLED.h>
#include <algorithm>
#include "KeypadParams.h"
#include "main.h"
#include "Effect.h"

class TicTacToe : public Effect
{
private:
    enum GameState
    {
        ready,
        playing,
        score
    };

    enum TttObject
    {
        // Starts at -1 for score calculations
        user = -1,
        tie,
        ai,
        empty
    };

    float delayElapsed;
    TttObject tttWinner;
    TttObject tttBoard[HEIGHT][WIDTH];
    GameState tttState;
    TttObject tttCurrentPlayer;

public:
    void Load() override
    {
        std::fill_n(&tttBoard[0][0], WIDTH * HEIGHT, empty);
        tttState = ready;
        tttCurrentPlayer = user;
        tttWinner = empty;
        delayElapsed = 0.0f;
    }

    void Update(float secondsElapsed) override
    {
        rgb.Clear();

        switch (tttState)
        {
        case ready:
            delayElapsed += secondsElapsed;

            if (delayElapsed >= 4.0f)
            {
                delayElapsed = 0.0f;
                tttState = playing;
                // Fall through...
            }
            else
            {
                for (uint8_t i = 0; i < min((int)delayElapsed, 3) + 1; i++)
                    for (uint8_t j = 0; j < HEIGHT; j++)
                        rgb.SetColor(j * WIDTH + i, CHSV(HUE_AQUA, 255, rgb.GetBrightness()));
                break;
            }

        case playing:
            tttWinner = TttCheckWinner();
            if (tttWinner != empty)
            {
                // Only start the timer when there is a winner,
                // if so show the score after 5 seconds
                delayElapsed += secondsElapsed;
                if (delayElapsed > 5.0f)
                {
                    delayElapsed = 0.0f;
                    tttState = score;
                    // Fall through...
                }
                else
                    break;
            }
            else
            {
                if (tttCurrentPlayer == ai)
                {
                    int8_t bestScore = -128;
                    uint8_t moveX = 0;
                    uint8_t moveY = 0;
                    for (uint8_t j = 0; j < HEIGHT; j++)
                        for (uint8_t i = 0; i < WIDTH; i++)
                            if (tttBoard[j][i] == empty)
                            {
                                tttBoard[j][i] = ai;
                                int8_t score = TttGetMinimaxBestscore(false);
                                tttBoard[j][i] = empty;
                                if (score > bestScore)
                                {
                                    bestScore = score;
                                    moveX = j;
                                    moveY = i;
                                }
                            }
                    tttBoard[moveX][moveY] = ai;
                    tttCurrentPlayer = user;
                }

                FastLED.clear();

                for (uint8_t j = 0; j < HEIGHT; j++)
                    for (uint8_t i = 0; i < WIDTH; i++)
                        switch (tttBoard[j][i])
                        {
                        case empty:
                            break;
                        case ai:
                            rgb.DrawPixel2d(j, i, CHSV(HUE_BLUE, 255, rgb.GetBrightness()));
                            break;
                        case user:
                            rgb.DrawPixel2d(j, i, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                            break;
                        case tie: // Not used for board
                            break;
                        }
                break;
            }

        case score:
            delayElapsed += secondsElapsed;
            if (delayElapsed > 5.0f)
            {
                std::fill_n(&tttBoard[0][0], WIDTH * HEIGHT, empty);
                tttState = ready;
                tttCurrentPlayer = user;
                tttWinner = empty;
                delayElapsed = 0.0f;
                break;
            }

            switch (tttWinner)
            {
            case ai:
                rgb.DrawPixel2d(0, 0, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                rgb.DrawPixel2d(0, 3, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                rgb.DrawPixel2d(1, 1, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                rgb.DrawPixel2d(1, 2, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                rgb.DrawPixel2d(2, 0, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                rgb.DrawPixel2d(2, 3, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                break;
            case user:
                rgb.DrawPixel2d(0, 1, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                rgb.DrawPixel2d(0, 2, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                rgb.DrawPixel2d(1, 0, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                rgb.DrawPixel2d(1, 3, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                rgb.DrawPixel2d(2, 1, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                rgb.DrawPixel2d(2, 2, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                break;
            case tie:
                for (uint8_t i = 0; i < WIDTH; i++)
                    rgb.DrawPixel2d(1, i, CHSV(HUE_RED, 255, rgb.GetBrightness()));
                break;
            case empty: // Winner won't be empty in there
                break;
            }
            break;
        }
    }

    void HandleKey(uint8_t currentState, uint8_t keyX, uint8_t keyY) override
    {
        if (tttCurrentPlayer == user)
            if (tttBoard[keyY][keyX] == empty)
            {
                tttBoard[keyY][keyX] = user;
                tttCurrentPlayer = ai;
            }
    }

    std::string GetName() override { return "ticTacToe"; }

private:
    TttObject TttCheckWinner()
    {
        // Horizontal
        for (uint8_t i = 0; i < WIDTH - 2; i++)
            for (uint8_t j = 0; j < HEIGHT; j++)
                if (tttBoard[j][i] != empty && tttBoard[j][i] == tttBoard[j][i + 1] && tttBoard[j][i + 1] == tttBoard[j][i + 2])
                    return tttBoard[j][i];
        // Vertical
        for (uint8_t i = 0; i < WIDTH; i++)
            if (tttBoard[0][i] != empty && tttBoard[0][i] == tttBoard[1][i] && tttBoard[1][i] == tttBoard[2][i])
                return tttBoard[0][i];
        // Diagonal
        for (uint8_t i = 0; i < WIDTH - 2; i++)
        {
            if (tttBoard[0][i] != empty && tttBoard[0][i] == tttBoard[1][i + 1] && tttBoard[1][i + 1] == tttBoard[2][i + 2])
                return tttBoard[0][i];
            if (tttBoard[2][i] != empty && tttBoard[2][i] == tttBoard[1][i + 1] && tttBoard[1][i + 1] == tttBoard[1][i + 2])
                return tttBoard[2][i];
        }
        // Check for tie, no empty slots
        uint8_t emptySlots = 0;
        for (uint8_t j = 0; j < HEIGHT; j++)
            for (uint8_t i = 0; i < WIDTH; i++)
                if (tttBoard[j][i] == empty)
                    emptySlots++;
        if (emptySlots == 0)
            return tie;
        return empty;
    }

    int8_t TttGetMinimaxBestscore(bool isMaximizing)
    {
        TttObject result = TttCheckWinner();
        if (result != empty)
            return (int8_t)result;

        if (isMaximizing)
        {
            int8_t bestScore = -128;
            for (uint8_t j = 0; j < HEIGHT; j++)
                for (uint8_t i = 0; i < WIDTH; i++)
                    if (tttBoard[j][i] == empty)
                    {
                        tttBoard[j][i] = ai;
                        int8_t score = TttGetMinimaxBestscore(false);
                        tttBoard[j][i] = empty;
                        bestScore = max(score, bestScore);
                    }
            return bestScore;
        }
        else
        {
            int8_t bestScore = 127;
            for (uint8_t j = 0; j < HEIGHT; j++)
                for (uint8_t i = 0; i < WIDTH; i++)
                    if (tttBoard[j][i] == empty)
                    {
                        tttBoard[j][i] = user;
                        int8_t score = TttGetMinimaxBestscore(true);
                        tttBoard[j][i] = empty;
                        bestScore = min(score, bestScore);
                    }
            return bestScore;
        }
    }
};