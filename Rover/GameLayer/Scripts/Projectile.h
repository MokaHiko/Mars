#ifndef PROJECTILE_H
#define PROJECTILE_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>
#include <Math/Math.h>

struct ProjectileProperties
{
    float damage = 10.0f;
    float speed = 5.0f;
    float life_span = 5.0f;
    mrs::Vector2 direction = {0,1};
};

class Projectile : public mrs::ScriptableEntity
{
public:
    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;

    virtual void OnCollisionEnter2D(mrs::Entity other) override;
private:

};

#endif