#ifndef GRAVGUN_H
#define GRAVGUN_H

#pragma once

#include "Weapon.h"

class GravGun : public Weapon
{
public:
    GravGun();
    ~GravGun();

    virtual void FireStrongProjectile(float hold_time) override;
private:

};

#endif