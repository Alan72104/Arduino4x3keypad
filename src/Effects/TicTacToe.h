#include <FastLED.h>
#include <algorithm>
#include "Effect.h"
#include "KeypadParams.h"
#include "main.h"

class TicTacToe : public Effect
{
private:
    enum GameState
    {
        ready,
        playing,
        score
    };

    enum Object
    {
        // Starts at -1 for score calculations
        user = -1,
        tie,
        ai,
        empty
    };

    float delayElapsed;
    Object winner;
    Object board[HEIGHT][WIDTH];
    GameState state;
    Object currentPlayer;

public:
    void Load() override
    {
        std::fill_n(&board[0][0], WIDTH * HEIGHT, empty);
        state = ready;
        currentPlayer = user;
        winner = empty;
        delayElapsed = 0.0f;
    }

    void Update(float secondsElapsed) override
    {
        rgb.Clear();

        switch (state)
        {
            case ready:
                delayElapsed += secondsElapsed;

                if (delayElapsed >= 4.0f)
                {
                    delayElapsed = 0.0f;
                    state = playing;
                    // Fall through...
                }
                else
                {
                    for (uint8_t i = 0; i < min((int)delayElapsed, 3) + 1; i++)
                        for (uint8_t j = 0; j < HEIGHT; j++)
                            rgb.Draw(j * WIDTH + i, CHSV(HUE_AQUA, 255, rgb.GetBrightness()));
                    break;
                }

            case playing:
                winner = CheckWinner();
                if (winner != empty)
                {
                    // Only start the timer when there is a winner,
                    // if so show the score after 5 seconds
                    delayElapsed += secondsElapsed;
                    if (delayElapsed > 5.0f)
                    {
                        delayElapsed = 0.0f;
                        state = score;
                        // Fall through...
                    }
                    else
                        break;
                }
                else
                {
                    if (currentPlayer == ai)
                    {
                        int8_t bestScore = -128;
                        uint8_t moveX = 0;
                        uint8_t moveY = 0;
                        for (uint8_t j = 0; j < HEIGHT; j++)
                            for (uint8_t i = 0; i < WIDTH; i++)
                                if (board[j][i] == empty)
                                {
                                    board[j][i] = ai;
                                    int8_t score = GetMinimaxBestscore(false);
                                    board[j][i] = empty;
                                    if (score > bestScore)
                                    {
                                        bestScore = score;
                                        moveX = j;
                                        moveY = i;
                                    }
                                }
                        board[moveX][moveY] = ai;
                        currentPlayer = user;
                    }

                    rgb.Clear();

                    for (uint8_t j = 0; j < HEIGHT; j++)
                        for (uint8_t i = 0; i < WIDTH; i++)
                            switch (board[j][i])
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
                    std::fill_n(&board[0][0], WIDTH * HEIGHT, empty);
                    state = ready;
                    currentPlayer = user;
                    winner = empty;
                    delayElapsed = 0.0f;
                    break;
                }

                switch (winner)
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

    void OnKeyPressed(uint8_t state, uint8_t keyX, uint8_t keyY) override
    {
        if (currentPlayer == user)
            if (board[keyY][keyX] == empty)
            {
                board[keyY][keyX] = user;
                currentPlayer = ai;
            }
    }

    bool IsGameEffect() override { return true; }

private:
    Object CheckWinner()
    {
        // Horizontal
        for (uint8_t i = 0; i < WIDTH - 2; i++)
            for (uint8_t j = 0; j < HEIGHT; j++)
                if (board[j][i] != empty && board[j][i] == board[j][i + 1] && board[j][i + 1] == board[j][i + 2])
                    return board[j][i];
        // Vertical
        for (uint8_t i = 0; i < WIDTH; i++)
            if (board[0][i] != empty && board[0][i] == board[1][i] && board[1][i] == board[2][i])
                return board[0][i];
        // Diagonal
        for (uint8_t i = 0; i < WIDTH - 2; i++)
        {
            if (board[0][i] != empty && board[0][i] == board[1][i + 1] && board[1][i + 1] == board[2][i + 2])
                return board[0][i];
            if (board[2][i] != empty && board[2][i] == board[1][i + 1] && board[1][i + 1] == board[1][i + 2])
                return board[2][i];
        }
        // Check for tie, no empty slots
        uint8_t emptySlots = 0;
        for (uint8_t j = 0; j < HEIGHT; j++)
            for (uint8_t i = 0; i < WIDTH; i++)
                if (board[j][i] == empty)
                    emptySlots++;
        if (emptySlots == 0)
            return tie;
        return empty;
    }

    int8_t GetMinimaxBestscore(bool isMaximizing)
    {
        Object result = CheckWinner();
        if (result != empty)
            return (int8_t)result;

        if (isMaximizing)
        {
            int8_t bestScore = -128;
            for (uint8_t j = 0; j < HEIGHT; j++)
                for (uint8_t i = 0; i < WIDTH; i++)
                    if (board[j][i] == empty)
                    {
                        board[j][i] = ai;
                        int8_t score = GetMinimaxBestscore(false);
                        board[j][i] = empty;
                        bestScore = max(score, bestScore);
                    }
            return bestScore;
        }
        else
        {
            int8_t bestScore = 127;
            for (uint8_t j = 0; j < HEIGHT; j++)
                for (uint8_t i = 0; i < WIDTH; i++)
                    if (board[j][i] == empty)
                    {
                        board[j][i] = user;
                        int8_t score = GetMinimaxBestscore(true);
                        board[j][i] = empty;
                        bestScore = min(score, bestScore);
                    }
            return bestScore;
        }
    }
};