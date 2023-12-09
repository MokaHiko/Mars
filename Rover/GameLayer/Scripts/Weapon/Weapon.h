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
    virtual void OnUpdate(float dt) final;

    void Equip(Ship* ship);
    void Fire();
protected:
    virtual void OnEquip() {};
    virtual void OnUnEquip() {};

    virtual void FireWeakProjectile() = 0;
    virtual void FireStrongProjectile(float hold_time) = 0;

    Ship* Owner() {return _ship;}

    float _long_hold_threash_hold = 1.0f;

    int _ammo = 100;
    float _fire_rate = 0.35f;
    int _cannon_side = 1;
private:
    float _hold_time = 0.0f;
    float _time_since_last_fire = 0.0f;

    Ship* _ship = {};
};

#endif