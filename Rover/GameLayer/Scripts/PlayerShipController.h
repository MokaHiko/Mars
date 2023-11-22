#ifndef PLAYERSHIPCONTROLLER_H
#define PLAYERSHIPCONTROLLER_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

class PlayerShipController : public mrs::ScriptableEntity
{
public:
    virtual void OnCreate() override;
    virtual void OnUpdate(float dt) override;
private:
    mrs::Entity _ship;
};

#endif