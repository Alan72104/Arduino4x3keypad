#ifndef EFFECT_H
#define EFFECT_H

#include <string>

class Effect
{
protected:
    static const uint8_t rainbowHues[7];

public:
    virtual void Load() = 0;
    virtual void Unload() {};
    virtual void Update(float secondsElapsed) = 0;
    virtual void OnKeyPressed(uint8_t state, uint8_t keyX, uint8_t keyY) {};
    virtual std::string GetName() = 0;
    virtual bool IsGameEffect() { return false; };
};

#endif