#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H

#include "Effect.h"

enum EffectType : uint8_t
{
    rainbow,
    staticFade,
    staticRainbow,
    splash,
    staticLight,
    breathing,
    fractionalDrawingTest2d,
    spinningRainbow,
    ripple,
    antiRipple,
    stars,
    raindrop,
    snake,
    shootingParticles,
    fire,
    whacAMole,
    ticTacToe,
    bullsNCows
};

class EffectManager
{
private:
    void CreateEffect(uint8_t i);

    Effect* currentEffect;
    uint8_t currentEffectNum;
    static const uint8_t effectCount = 18;
    float secondsElapsed;
    float effectSpeedScale;

public:
    void Init();
    void SetEffect(uint8_t i);
    void NextEffect();
    void UpdateEffect();
    void IncreaseEffectSpeed(float amount = 0.2f);
    void DecreaseEffectSpeed(float amount = 0.2f);
    void HandleKey(uint8_t currentState, uint8_t keyX, uint8_t keyY);
    Effect* GetCurrentEffect();
    EffectType GetCurrentEffectType();
    bool IsCurrentEffectGameEffect();
};

#endif