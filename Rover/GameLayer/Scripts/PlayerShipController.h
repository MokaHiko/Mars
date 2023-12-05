#ifndef PLAYERSHIPCONTROLLER_H
#define PLAYERSHIPCONTROLLER_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

#include "Ship.h"

class PlayerShipController : public mrs::ScriptableEntity
{
public:
    virtual void OnStart() override;
    virtual void OnCreate() override;
    virtual void OnUpdate(float dt) override;
private:
    Ship* _ship = nullptr;
};

#endif