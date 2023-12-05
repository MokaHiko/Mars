#ifndef WEAPON_H
#define WEAPON_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>
#include <Math/Math.h>

#include "Projectile/Projectile.h"

class Ship;
class Weapon : public mrs::ScriptableEntity
{
public:
    virtual void OnStart() final;
    virtual void OnUpdate(float dt) final;

    void Equip(Ship* ship);
    void Fire();
protected:
    virtual void OnEquip() {};
    virtual void OnUnEquip() {};

    virtual mrs::Entity CreateWeakProjectile() = 0;
    virtual mrs::Entity CreateStrongProjectile(float hold_time) = 0;

    Ship* Owner() {return _ship;}

    float _long_hold_threash_hold = 1.0f;
private:
    float _hold_time = 0.0f;
    float _time_since_last_fire = 0.0f;
private:
    int _ammo = 100;
    float _fire_rate = 0.15f;
    int _cannon_side = 1;
    Ship* _ship = {};
};

#endif