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
    void Fire(int alternate_fire = 0);
protected:
    virtual void OnEquip() {};
    virtual void OnUnEquip() {};
    
    virtual void FireWeakProjectile() = 0;
    virtual void FireStrongProjectile(float hold_time) = 0;

    Ship* Owner() {return _ship;}

    float _long_hold_threash_hold = 1.0f;

    int _primary_ammo = 100;
    int _secondary_ammo = 10;
    float _primary_fire_rate = 0.35f;
    float _secondary_fire_rate = 1.0f;
    int _cannon_side = 1;
private:
    float _hold_time = 0.0f;
    float _time_since_last_primary_fire = 0.0f;
    float _time_since_last_secondary_fire = 0.0f;

    Ship* _ship = {};
};

#endif