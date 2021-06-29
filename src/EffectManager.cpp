#include <string>
#include "EffectManager.h"
#include "Effects/Effects.h"
#include "Keypad.h"
#include "main.h"

void EffectManager::Init() { CreateEffect(0); effectSpeedScale = 1.0f; }

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
    GetCurrentEffect()->Unload();
    delete GetCurrentEffect();

    if (++currentEffectNum >= effectCount)
        currentEffectNum = 0;

    CreateEffect(currentEffectNum);
    GetCurrentEffect()->Load();

    if (IsCurrentEffectGameEffect())
        keypad.ResetAllStateForDriver();
}

const uint8_t EffectManager::effectCount = 17;

void EffectManager::CreateEffect(uint8_t i)
{
    switch (i)
    {
        case 0:  currentEffect = new Rainbow();                 break;
        case 1:  currentEffect = new StaticRainbow();           break;
        case 2:  currentEffect = new Splash();                  break;
        case 3:  currentEffect = new StaticLight();             break;
        case 4:  currentEffect = new Breathing();               break;
        case 5:  currentEffect = new FractionalDrawingTest2d(); break;
        case 6:  currentEffect = new SpinningRainbow();         break;
        case 7:  currentEffect = new Ripple();                  break;
        case 8:  currentEffect = new AntiRipple();              break;
        case 9:  currentEffect = new Stars();                   break;
        case 10: currentEffect = new Raindrop();                break;
        case 11: currentEffect = new Snake();                   break;
        case 12: currentEffect = new ShootingParticles();       break;
        case 13: currentEffect = new Fire();                    break;
        case 14: currentEffect = new WhacAMole();               break;
        case 15: currentEffect = new TicTacToe();               break;
        case 16: currentEffect = new BullsNCows();              break;
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

void EffectManager::IncreaseEffectSpeed() { effectSpeedScale = min(effectSpeedScale + 0.2f, 8.0f); }
void EffectManager::DecreaseEffectSpeed() { effectSpeedScale = max(1.0f, effectSpeedScale - 0.2f); }

void EffectManager::HandleKey(uint8_t currentState, uint8_t keyX, uint8_t keyY)
{
    GetCurrentEffect()->OnKeyPressed(currentState, keyX, keyY);
}

Effect* EffectManager::GetCurrentEffect() { return currentEffect; }

std::string EffectManager::GetCurrentEffectName() { return GetCurrentEffect()->GetName(); }

bool EffectManager::IsCurrentEffectGameEffect() { return GetCurrentEffect()->IsGameEffect(); }