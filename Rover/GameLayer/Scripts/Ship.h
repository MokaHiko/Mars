#ifndef SHIP_H
#define SHIP_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

class Ship : public mrs::ScriptableEntity
{
public:
    Ship();
    ~Ship();

    virtual void OnCreate() override;
    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;

    virtual void OnCollisionEnter2D(mrs::Entity other) override;

private:
    float Lerp(float a, float b, float t)
    {
        return (a * (1 - t)) + (b * t);
    }
};

#endif