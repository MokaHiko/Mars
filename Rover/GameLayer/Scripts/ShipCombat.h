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

enum class ShipCombatState
{
    Idle,
    AutoCombat,
    Manual
};

class ShipCombat : public mrs::ScriptableEntity
{
public:
    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;

    void ProcessInput();
public:
    void FireProjectile(mrs::Entity target);
    void ScanTargets();

    // Weapon
    float _fire_rate = 0.15f;
    int _cannon_side = 1;

    std::vector<mrs::Entity> _targets;
private:
    ShipCombatState _state = ShipCombatState::Idle;
    Ship* _ship = nullptr;
    mrs::Entity _ship_entity;
};

class AutoCombatProcess: public mrs::Process
{
public:
    AutoCombatProcess(ShipCombat& combat);
    virtual void OnUpdate(float dt) override;

private:
    float _time_since_last_fire = 0.0f;
    std::vector<mrs::Entity>::iterator _target_it = {};
private:
    ShipCombat& _ship_combat;
    float _time = 0;
};

#endif