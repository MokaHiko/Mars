#ifndef SPAWNER_H
#define SPAWNER_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

#include "Unit.h"

class Spawner : public mrs::ScriptableEntity
{
public:
    virtual void OnStart() override 
    {
    }

    virtual void OnUpdate(float dt) override
    {
        _timer += dt;
        if(_timer > _spawn_rate)
        {
            SpawnUnit();
            _timer = 0;
        }
    }

    void SpawnUnit()
    {
        auto& e = Instantiate("box");

        static int ctr = 0;
        if(ctr > 8)
        {
            ctr = 0;
        }

        e.GetComponent<mrs::Transform>().position = GetComponent<mrs::Transform>().position + glm::vec3(ctr, ctr, 0.0f);
        ctr++;

        e.AddComponent<mrs::RigidBody2D>();
        e.AddComponent<mrs::RenderableObject>();
        e.AddComponent<mrs::Script>().Bind<Unit>();
    }

private:
    float _timer = 0.0f;
    float _spawn_rate  = 0.5f;
};

#endif