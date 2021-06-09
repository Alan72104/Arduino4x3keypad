#ifndef EFFECT_H
#define EFFECT_H

class Effect
{
public:
    virtual void Load() = 0;
    virtual void Update() = 0;
};

#endif