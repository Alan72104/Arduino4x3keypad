#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H

#include <string>
#include "Effect.h"

class EffectManager
{
private:
    void CreateEffect(uint8_t i);

    Effect* currentEffect;
    uint8_t currentEffectNum;
    static const uint8_t effectCount;
    float secondsElapsed;
    float effectSpeedScale;

public:
    void Init();
    void SetEffect(uint8_t i);
    void NextEffect();
    void UpdateEffect();
    void IncreaseEffectSpeed(float amount = 2.0f);
    void DecreaseEffectSpeed(float amount = 2.0f);
    void HandleKey(uint8_t currentState, uint8_t keyX, uint8_t keyY);
    Effect* GetCurrentEffect();
    std::string GetCurrentEffectName();
    bool IsCurrentEffectGameEffect();
};

#endif