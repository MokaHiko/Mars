#ifndef UNIT_H
#define UNIT_H

#pragma once

#include <ECS/ScriptableEntity.h>

class Unit : public mrs::ScriptableEntity
{
public:
    virtual void OnCreate() override {}
    virtual void OnStart() override {}

    virtual void OnUpdate(float dt) override
    {
        if(!_to_destroy)
        {
            return;
        }

        _timer += dt;
        if(_timer > _death_timer)
        {
            QueueDestroy();
        }
    }

    virtual void OnCollisionEnter2D(mrs::Entity other) override
    {
        _to_destroy = true;
        _death_timer = static_cast<float>(rand() % 20);
    }

private:
    bool _to_destroy = false;
    float _death_timer = 0.0f;
    float _timer = 0;
};

#endif