#ifndef TRACEREFFECT_H
#define TRACEREFFECT_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

#include "Effect.h"

class TracerEffect: public Effect
{
public:
    virtual void Init() override;
    virtual void OnUpdate(float dt) override;

    mrs::Entity target = {};

    mrs::Vector3 p1 = {};
    mrs::Vector3 p2 = {};

    float duration = 0.0f;
    float time_elapsed = 0.0f;
};



#endif