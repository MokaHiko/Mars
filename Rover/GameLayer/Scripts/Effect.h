#ifndef EFFECT_H
#define EFFECT_H

#pragma once
#include <ECS/ScriptableEntity.h>

class Effect : public mrs::ScriptableEntity
{
public:
    float _life_time = 6.0f;

    virtual void OnUpdate(float dt)
    {
        _life_time -= dt;

        if (_life_time <= 0)
        {
            QueueDestroy();
        }
    }
};

#endif