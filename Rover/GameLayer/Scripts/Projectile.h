#ifndef PROJECTILE_H
#define PROJECTILE_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

class Projectile : public mrs::ScriptableEntity
{
public:
    float _life_span = 1.25f;
    float _time_alive = 0.0f;

    mrs::ParticleSystem* _collision_particle_system;
    mrs::RigidBody2D* _rb;

    virtual void OnCreate() override {}
    virtual void OnStart() override 
    {
        _collision_particle_system = &GetComponent<mrs::ParticleSystem>();
        _rb = &GetComponent<mrs::RigidBody2D>();
    }

    virtual void OnUpdate(float dt) override 
    {
        _time_alive += dt;
        if(_time_alive >= _life_span)
        {
            QueueDestroy();
        }
    }

    virtual void OnCollisionEnter(mrs::Entity other) override
    {
        _collision_particle_system->Play();
        _rb->SetVelocity(glm::vec2{0.0f});
        _rb->SetGravityScale(0.0f);

        //QueueDestroy();
    }
private:

};

#endif