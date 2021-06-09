#include <Arduino.h>
#include "EffectManager.h"

void EffectManager::AddEffect(Effect* effect) { effects.push_back(effect); }

void EffectManager::SetEffect(uint8_t i)
{
    if (i < effects.size())
        currentEffectNum = i;

}

void EffectManager::NextEffect()
{
    if (++currentEffectNum == effects.size())
        currentEffectNum = 0;
}

Effect* EffectManager::GetCurrentEffect() { return effects[currentEffectNum]; }

std::string EffectManager::GetCurrentEffectName() { return effects[currentEffectNum]->GetName(); };