#ifndef PROJECTILE_H
#define PROJECTILE_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

#include "Effect.h"

class Projectile : public mrs::ScriptableEntity
{
public:
    float _life_span = 5.0f;
    float _time_alive = 0.0f;

    mrs::ParticleSystem *_collision_particle_system;
    mrs::RigidBody2D *_rb;

    virtual void OnCreate() override {}
    virtual void OnStart() override
    {
        _collision_particle_system = &GetComponent<mrs::ParticleSystem>();
        _rb = &GetComponent<mrs::RigidBody2D>();
    }

    virtual void OnUpdate(float dt) override
    {
        _time_alive += dt;
        if (_time_alive >= _life_span)
        {
            QueueDestroy();
        }
    }

    virtual void OnCollisionEnter(mrs::Entity other) override
    {
        _rb->SetVelocity(glm::vec2{ 0.0f });
        _rb->SetGravityScale(0.0f);

        mrs::Entity collision_effect = Insantiate("Collision Particles");
        collision_effect.GetComponent<mrs::Transform>().position = GetComponent<mrs::Transform>().position;

        auto &script_component = collision_effect.AddComponent<mrs::Script>();
        script_component.Bind<Effect>();

        auto &particles = collision_effect.AddComponent<mrs::ParticleSystem>();
        particles.max_particles = 12;
        particles.velocity = glm::vec2(750.0f);
        particles.life_time = 10.0f;
        particles.particle_size = 0.15f;
        particles.emission_rate = 254.0f;
        particles.emission_shape = mrs::EmissionShape::Circle;
        particles.running = true;

        particles.color_1 = glm::vec4(255, 240, 0, 255);
        particles.color_2 = glm::vec4(255, 240, 0, 0);

        QueueDestroy();
    }
private:

};

#endif