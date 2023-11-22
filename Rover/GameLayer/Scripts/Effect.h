#ifndef EFFECT_H
#define EFFECT_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

struct EffectProperties
{
    float duration;
};

class Effect : public mrs::ScriptableEntity
{
public:
    virtual void OnStart() override;
private:
};

#endif