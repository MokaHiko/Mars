#ifndef SHIPCOMBAT_H
#define SHIPCOMBAT_H

#pragma once


#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>
#include <Scripting/Process.h>

#include "Ship.h"
#include "Weapon/Weapon.h"

struct Target
{
    int marker;
};

enum class ShipCombatState
{
    Idle,
    AutoCombat,
    ManualCombat
};

class ShipCombat : public mrs::ScriptableEntity
{
public:
    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;

    void SwitchCombatMode();
    void EquipWeapon(Weapon* weapon);
public:
    void ScanTargets();

    // Manual combat
    void ManualFire();

    std::vector<mrs::Entity> _targets;
    Ship* _ship = nullptr;
private:
    Weapon* _current_weapon = nullptr;

    ShipCombatState _state = ShipCombatState::Idle;
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