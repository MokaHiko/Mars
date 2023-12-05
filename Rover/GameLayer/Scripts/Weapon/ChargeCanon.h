#ifndef CHARGECANON_H
#define CHARGECANON_H

#pragma once

#include "Weapon.h"

class ChargeCanon : public Weapon
{
public:
    virtual mrs::Entity CreateWeakProjectile() override;
    virtual mrs::Entity CreateStrongProjectile(float hold_time) override;

    virtual void OnEquip() override;
    virtual void OnUnEquip() override;
private:
};

#endif