#ifndef UNIT_H
#define UNIT_H

#pragma once

#include <ECS/ScriptableEntity.h>

class Player;
class Unit : public mrs::ScriptableEntity
{
public:
    virtual void OnCreate() override;
    virtual void OnStart() override;

    virtual void OnUpdate(float dt) override;

    virtual void OnCollisionEnter2D(mrs::Entity other) override;
private:
    float _duration = 2.0f;
};

#endif