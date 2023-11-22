#ifndef SHIP_H
#define SHIP_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

struct ShipResources
{
    uint32_t scrap_metal = 0;
    uint32_t credit = 0;
};

class Ship : public mrs::ScriptableEntity
{
public:
    virtual void OnCreate() override;
    virtual void OnUpdate(float dt) override;

    virtual void OnCollisionEnter2D(mrs::Entity other) override;

public:
    void TakeDamage(float damage);
    void Die();
public:
    float _health = 100.0f;
};

#endif