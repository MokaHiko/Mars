#ifndef CHARGECANON_H
#define CHARGECANON_H

#pragma once

#include "Weapon.h"
#include <Toolbox/RandomToolBox.h>

class ChargeCanon : public Weapon
{
public:
    virtual void OnStart() override;

    virtual void FireWeakProjectile() override;
    virtual void FireStrongProjectile(float hold_time) override;

    virtual void OnEquip() override;
    virtual void OnUnEquip() override;

    static tbx::PRNGenerator<float> random_gen;
private:
    mrs::Entity BulletMuzzleFlash();
};

#endif