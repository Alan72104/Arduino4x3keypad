#ifndef EFFECT_H
#define EFFECT_H

#include <string>

class Effect
{
public:
    virtual void Load() = 0;
    virtual void Update() = 0;
    virtual std::string GetName() = 0;
};

#endif