#include <string>
#include "EffectManager.h"
#include "Effects/Effects.h"
#include "Keypad.h"
#include "main.h"

void EffectManager::Init()
{
    CreateEffect(0);
    GetCurrentEffect()->Load();
    effectSpeedScale = 1.0f;
}

void EffectManager::SetEffect(uint8_t i)
{
    if (i >= effectCount) return;

    GetCurrentEffect()->Unload();
    delete GetCurrentEffect();

    currentEffectNum = i;
    
    CreateEffect(currentEffectNum);
    GetCurrentEffect()->Load();

    if (IsCurrentEffectGameEffect())
        keypad.ResetAllStateForDriver();
}

void EffectManager::NextEffect()
{
    if (++currentEffectNum >= effectCount)
        currentEffectNum = 0;

    SetEffect(currentEffectNum);
}

void EffectManager::CreateEffect(uint8_t i)
{
    switch (i)
    {
        case 0:  currentEffect = new Rainbow();                 break;
        case 1:  currentEffect = new StaticFade();              break;
        case 2:  currentEffect = new StaticRainbow();           break;
        case 3:  currentEffect = new Splash();                  break;
        case 4:  currentEffect = new StaticLight();             break;
        case 5:  currentEffect = new Breathing();               break;
        case 6:  currentEffect = new FractionalDrawingTest2d(); break;
        case 7:  currentEffect = new SpinningRainbow();         break;
        case 8:  currentEffect = new Ripple();                  break;
        case 9:  currentEffect = new AntiRipple();              break;
        case 10: currentEffect = new Stars();                   break;
        case 11: currentEffect = new Raindrop();                break;
        case 12: currentEffect = new Snake();                   break;
        case 13: currentEffect = new ShootingParticles();       break;
        case 14: currentEffect = new Fire();                    break;
        case 15: currentEffect = new WhacAMole();               break;
        case 16: currentEffect = new TicTacToe();               break;
        case 17: currentEffect = new BullsNCows();              break;
    }
}

void EffectManager::UpdateEffect()
{
    static uint32_t lastEffectUpdate = 0ul;
    if (micros() - lastEffectUpdate < 33333 /* 30 fps */) return;
    secondsElapsed = (micros() - lastEffectUpdate) / 1000.0f / 1000.0f;
    lastEffectUpdate = micros();

    GetCurrentEffect()->Update(secondsElapsed * effectSpeedScale);
}

void EffectManager::IncreaseEffectSpeed(float amount) { effectSpeedScale = min(effectSpeedScale + amount, 40.0f); }
void EffectManager::DecreaseEffectSpeed(float amount) { effectSpeedScale = max(0.1f, effectSpeedScale - amount); }

void EffectManager::HandleKey(uint8_t currentState, uint8_t keyX, uint8_t keyY)
{
    GetCurrentEffect()->OnKeyPressed(currentState, keyX, keyY);
}

Effect* EffectManager::GetCurrentEffect() { return currentEffect; }
EffectType EffectManager::GetCurrentEffectType() { return (EffectType)currentEffectNum; }

bool EffectManager::IsCurrentEffectGameEffect() { return GetCurrentEffect()->IsGameEffect(); }