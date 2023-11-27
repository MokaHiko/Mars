#ifndef PLANET_H
#define PLANET_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>
#include <Math/Math.h>

struct PlanetProperties
{
    mrs::Vector3 axis_of_rotation;
    float rotation_rate;

    uint32_t population = 0;
};

class Planet : public mrs::ScriptableEntity
{
public:
    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;
private:
};

#endif