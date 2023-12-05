#ifndef SHIPMOVEMENT_H
#define SHIPMOVEMENT_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

#include "Ship.h"
#include <Scripting/Process.h>

class AnimateShipTurnProcess : public mrs::Process
{
public:
    AnimateShipTurnProcess(Ship* ship, float target_rotation, float duration);
    virtual ~AnimateShipTurnProcess() = default;

    virtual void OnUpdate(float dt) override;
private:
    float _initial_rotation = 0;
    float _target_rotation = 0;
    float _duration = 0;
    float _time_elapsed = 0;

    Ship* _ship = nullptr;
};

enum class ShipMovementState : uint16_t
{
    Idle,
    Accelerating,
};

class ShipMovement : public mrs::ScriptableEntity
{
public:
    void MoveTowards(mrs::Vector2 direction);
public:
    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;
    virtual void OnCollisionEnter2D(mrs::Entity other) override;

    Ship* _ship = nullptr;
private:
    void AnimateTurn(float w);
	int _last_dir = 0;
	int _dir = 0;
private:
    Ref<AnimateShipTurnProcess> _ship_turn_animation = nullptr;
    Ref<AnimateShipTurnProcess> _ship_thrusters_animation = nullptr;
private:
    ShipMovementState _movement_state = ShipMovementState::Idle;
    mrs::Entity _thrusters = {};
};

#endif