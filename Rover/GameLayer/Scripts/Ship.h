#ifndef SHIP_H
#define SHIP_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

struct ShipResources
{
    // Resources
    uint32_t scrap_metal = 10;
    uint32_t credit = 10;

    float fuel = 100.0f;
    float health = 100.0f;
};

struct ShipSpecs
{
    Ref<mrs::Model> model;
    float mass = 100.0f;

    float ms = 1.0f;
    float max_speed = 35.0f;
    float turn_speed = 120.0f;
};

class ShipMovement;
class ShipCombat;
class Ship : public mrs::ScriptableEntity
{
public:
    virtual void OnStart() override;
    virtual void OnCreate() override;
    virtual void OnUpdate(float dt) override;

    virtual void OnCollisionEnter2D(mrs::Collision& col) override;
public:
    ShipCombat& Combat();
    ShipMovement& Movement();
public:
    void TakeDamage(float damage);
    void Die();
public:
    ShipMovement* _ship_movement_script = nullptr;
    ShipCombat* _ship_combat_script = nullptr;

    mrs::Entity _ship_model = {};
    mrs::Entity _ship_thrusters = {};

};

#endif