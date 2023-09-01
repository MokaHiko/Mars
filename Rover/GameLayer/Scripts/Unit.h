#ifndef UNIT_H
#define UNIT_H

#pragma once

#include <ECS/ScriptableEntity.h>

class Unit : public mrs::ScriptableEntity
{
public:
    virtual void OnCreate() override {}
    virtual void OnStart() override {}

    virtual void OnUpdate(float dt) override;

    virtual void OnCollisionEnter2D(mrs::Entity other) override;
private:
    bool _to_destroy = false;
    float _death_timer = 0.0f;
    float _timer = 0;
};

#endif