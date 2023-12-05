#ifndef BULLET_H
#define BULLET_H

#pragma once

#include "Projectile.h"

class Bullet : public Projectile
{
public:
    virtual void Init() override;
    virtual void Launch() override;
};

#endif