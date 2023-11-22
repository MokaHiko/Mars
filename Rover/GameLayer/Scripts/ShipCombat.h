#ifndef SHIPCOMBAT_H
#define SHIPCOMBAT_H

#pragma once


#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>
#include <Scripting/Process.h>

#include "Projectile.h"
#include "Ship.h"

// class Volley: public mrs::Process
// {
// public:
//     Volley(float time);
//     virtual void OnUpdate(float dt) override;
// private:
//     float _time = 0;
// };

class ShipCombat : public mrs::ScriptableEntity
{
public:
    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;
public:
    void FireProjectile(mrs::Entity target);

    // Weapon
    float fire_rate = 0.15f;
    int cannon_side = 1;
private:
    std::vector<mrs::Entity> _targets;
private:
    Ship* _ship = nullptr;
    mrs::Entity _ship_entity;
};

#endif