#ifndef PROJECTILE_H
#define PROJECTILE_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>
#include <Math/Math.h>

class Projectile : public mrs::ScriptableEntity
{
public:
    virtual void OnCreate() final;
    virtual void OnStart() final;
    virtual void OnUpdate(float dt) final;

    virtual void OnCollisionEnter2D(mrs::Collision& col) override;
    virtual void Die();

    mrs::Entity _owner = {}; 
protected:
    // Basically bullet prefabs
    virtual void Init() = 0;

    virtual void Launch() = 0;

    float _damage = 10.0f;
    float _speed = 5.0f;
    float _life_span = 5.0f;
    mrs::Vector2 _direction = {0,0};
private:
    bool _tracking = false;
    float _time_alive = 0;
};

#endif