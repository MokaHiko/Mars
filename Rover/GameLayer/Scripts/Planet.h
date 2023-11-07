#ifndef PLANET_H
#define PLANET_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

class Planet : public mrs::ScriptableEntity
{
public:
    Planet();
    ~Planet();

    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;
private:
};

#endif