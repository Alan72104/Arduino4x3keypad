#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H

#include <Arduino.h>
#include <string>
#include "Effect.h"

class EffectManager
{
private:
    std::vector<Effect*> effects;
    uint8_t currentEffectNum;
    float secondsElapsed;

public:
    void AddEffect(Effect* effect);
    void SetEffect(uint8_t i);
    void NextEffect();
    void UpdateEffect();
    Effect* GetCurrentEffect();
    std::string GetCurrentEffectName();
};

#endif