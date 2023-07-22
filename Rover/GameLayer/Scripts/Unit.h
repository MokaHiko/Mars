#ifndef UNIT_H
#define UNIT_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

#include "PlayerController.h"
#include "Projectile.h"

class Unit : public mrs::ScriptableEntity
{
public:
    enum UnitState : uint16_t
    {
        Idle = 1 << 1,
        Moving = 1 << 2,
        Hit = 1 << 3,
        Dead = 1 << 4
    };

    float _health = 20.0f;
    float _movement_speed = 20.0f;
    UnitState _state = UnitState::Idle;

    mrs::Transform* _transform = nullptr;
    mrs::RigidBody2D* _rb = nullptr;

    mrs::Entity _player;
    mrs::Transform* _player_transform;

    // Moving state
    float _search_interval = 1.0f;
    float _time_moving = 0.0f;

    // Hit state
    float _time_stunned = 0.0f;
    float _stun_time = 2.0f;

    virtual void OnStart() 
    {
        _transform = &GetComponent<mrs::Transform>();
        _rb = &GetComponent<mrs::RigidBody2D>();

        _player = FindEntityWithScript<PlayerController>();
        _player_transform = &_player.GetComponent<mrs::Transform>();
    }

    virtual void OnUpdate(float dt) 
    {
        if (_state & UnitState::Idle)
        {
            IdleState();
        }
        else if (_state & UnitState::Moving)
        {
            MovingState(dt);
        }
        else if (_state & UnitState::Hit)
        {
            HitState(dt);
        }
        else if (_state & UnitState::Dead)
        {
            DeadState();
        }
    }

    virtual void OnCollisionEnter(mrs::Entity other) override 
    {
        if(other.HasComponent<mrs::Script>())
        {
            Projectile* projectile = dynamic_cast<Projectile*>(other.GetComponent<mrs::Script>().script);

            if(projectile)
            {
                auto& projectile_transform = other.GetComponent<mrs::Transform>();
				glm::vec3 dir = glm::normalize(_transform->position - projectile_transform.position);
				_rb->AddImpulse(dir * 50.0f);

                _health -= 5.0f;

                if(_health <= 0)
                {
                    _state = UnitState::Dead;
                    return;
                }

                _state = UnitState::Hit;
            }
        }
    }

    void HitState(float dt)
    {
        _time_stunned += dt;
        if (_time_stunned >= _stun_time)
        {
            _time_stunned = 0;
            _state = UnitState::Idle;
        }
    }

    void IdleState()
    {
        _state = UnitState::Moving;
    }

    void MovingState(float dt)
    {
        _time_moving += dt;

        if (_time_moving >= _search_interval)
        {
		    _time_moving = 0.0f;
        }

        // Check if can see player
        if (_player)
        {
			glm::vec3 dir = glm::normalize(_player_transform->position - _transform->position);
			_rb->SetVelocity(dir * _movement_speed);

			_state = UnitState::Moving;
        }
    }

    void DeadState()
    {
        QueueDestroy();
    }
};

#endif