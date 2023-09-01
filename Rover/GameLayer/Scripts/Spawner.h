#ifndef SPAWNER_H
#define SPAWNER_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

#include "Unit.h"

class Spawner : public mrs::ScriptableEntity
{
public:
    virtual void OnUpdate(float dt) override;

private:
    void SpawnUnit();

private:
    float _timer = 0.0f;
    float _spawn_rate  = 0.5f;
};

#endif