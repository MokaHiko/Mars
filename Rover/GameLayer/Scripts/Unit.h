#ifndef UNIT_H
#define UNIT_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

class Unit : public mrs::ScriptableEntity
{
public:
    mrs::ParticleSystem* _particles = nullptr;

    virtual void OnStart() 
    {
        _particles = &GetComponent<mrs::ParticleSystem>();
    }

    virtual void OnUpdate(float dt) {}

    virtual void OnCollisionEnter(mrs::Entity other) override {}
};

#endif