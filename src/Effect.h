#ifndef EFFECT_H
#define EFFECT_H

#include <string>

class Effect
{
public:
    virtual void Load() = 0;
    virtual void Update(float secondsElapsed) = 0;
    virtual void HandleKey(uint8_t currentState, uint8_t keyX, uint8_t keyY) {};
    virtual std::string GetName() = 0;
};

#endif