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

void EffectManager::UpdateEffect()
{
    static uint32_t lastEffectUpdate = 0ul;
    if (micros() - lastEffectUpdate < 33333 /* 30 fps */) return;
    secondsElapsed = (micros() - lastEffectUpdate) / 1000.0f / 1000.0f;
    lastEffectUpdate = micros();

    GetCurrentEffect()->Update(secondsElapsed);
}

void EffectManager::HandleKey(uint8_t currentState, uint8_t keyX, uint8_t keyY)
{
    GetCurrentEffect()->HandleKey(currentState, keyX, keyY);
}

Effect* EffectManager::GetCurrentEffect() { return effects[currentEffectNum]; }

std::string EffectManager::GetCurrentEffectName() { return effects[currentEffectNum]->GetName(); };