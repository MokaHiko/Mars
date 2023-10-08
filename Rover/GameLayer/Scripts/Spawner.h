#ifndef SPAWNER_H
#define SPAWNER_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

class Spawner : public mrs::ScriptableEntity
{
public:
    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;

private:
    void SpawnUnit();

private:
    uint32_t _max_units = 10;
    float _timer = 0.0f;
    float _spawn_rate  = 0.25f;
};

#endif