#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>
#include <Core/Input.h>
#include <glm/gtc/constants.hpp>

#include <SDL.h>

#include "Projectile.h"

class PlayerController : public mrs::ScriptableEntity
{
public:
    enum PlayerState : uint16_t
    {
        Uknown = 1 << 0,
        Idle = 1 << 1,
        Moving = 1 << 2,
    };

    // Handles
    mrs::Transform *_transform;
    mrs::RigidBody2D *_rb;
    mrs::ParticleSystem *_particles;

    // Player info
    PlayerState _state;
    float _movement_speed = 100.0f;
    float _turning_speed = 100.0f;

    float _fire_rate = 0.35f;
    float _projecitle_velocity = 25.0f;
    float _projectile_scale = 0.25f;
    float _current_cooldown_time = _fire_rate;

    virtual void OnStart() override
    {
        // Get handles
        _transform = &GetComponent<mrs::Transform>();
        _rb = &GetComponent<mrs::RigidBody2D>();
        //_particles = &GetComponent<mrs::ParticleSystem>();

        // Set initial state
        _state = PlayerState::Idle;
    }

    virtual void OnUpdate(float dt) override
    {
        if (_state & PlayerState::Idle)
        {
            IdleState(dt);
            return;
        }
    }

    virtual void OnCollisionEnter(mrs::Entity other) override
    {
    }

private:
    void IdleState(float dt)
    {
        if (mrs::Input::IsKeyDown(SDLK_q))
        {
            // Togle particles
            if(!_particles->running)
            {
                _particles->running = true;
            }
            else
            {
                _particles->Stop();
            }
        }

        // Keyboard 
        if (mrs::Input::IsKeyPressed(SDLK_w))
        {
            _rb->AddImpulse(glm::vec2(0.0f, 1.0f) * _movement_speed * dt);
        }
        else if (mrs::Input::IsKeyPressed(SDLK_s))
        {
            _rb->AddImpulse(glm::vec2(0.0f, -1.0f) * _movement_speed * dt);
        }

        if (mrs::Input::IsKeyPressed(SDLK_d))
        {
            _rb->AddImpulse(glm::vec2(1.0f, 0.0f) * _movement_speed * dt);
        }
        else if (mrs::Input::IsKeyPressed(SDLK_a))
        {
            _rb->AddImpulse(glm::vec2(-1.0f, 0.0f) * _movement_speed * dt);
        }

        // Mouse
        if(mrs::Input::IsMouseButtonPressed(SDL_BUTTON_RIGHT))
        {
            glm::vec2 mouse_pos = mrs::Input::GetMousePosition();
            float current_angle = glm::radians(_transform->rotation.z);

            float desired_angle = std::atan2(-mouse_pos.x, mouse_pos.y);
            desired_angle = glm::mix(current_angle, desired_angle, 5.0f * dt);

            if(desired_angle < -glm::pi<float>()) {desired_angle += glm::pi<float>() * 2;}
            if(desired_angle > glm::pi<float>()) {desired_angle -= glm::pi<float>() * 2;}

            _rb->body->SetTransform(_rb->body->GetPosition(), desired_angle);
            _rb->body->SetAngularVelocity(0);
        }

        _current_cooldown_time -= dt;
        if(_current_cooldown_time <= 0)
        {
            if(mrs::Input::IsMouseButtonPressed(SDL_BUTTON_LEFT))
            {
                auto e = Insantiate("Projectile");
                glm::vec2 target_dir = glm::normalize(mrs::Input::GetMousePosition());

                e.GetComponent<mrs::Transform>().position = _transform->position + glm::vec3(target_dir * 2.0f, _transform->position.z);
                e.GetComponent<mrs::Transform>().scale = glm::vec3(_projectile_scale);

                e.AddComponent<mrs::RenderableObject>();
                e.AddComponent<mrs::Script>().Bind<Projectile>();

                auto& rb = e.AddComponent<mrs::RigidBody2D>();
                rb.use_gravity = false;
                rb.AddImpulse(target_dir * _projecitle_velocity);
                rb.SetFixedRotation(true);

                auto& particles = e.AddComponent<mrs::ParticleSystem>();
                particles.max_particles = 12;
                particles.velocity = glm::vec2(750.0f);
                particles.life_time = 10.0f;
                particles.particle_size = 0.5f;
                particles.emission_rate = 254.0f;
                particles.emission_shape = mrs::EmissionShape::Circle;
                particles.running = false;

                particles.color_1 = glm::vec4(255,240,0,255);
                particles.color_2 = glm::vec4(255,240,0, 0);
                
                _current_cooldown_time = _fire_rate;
            }
        }
    }

    void MovingState()
    {

    }

private:
};

#endif