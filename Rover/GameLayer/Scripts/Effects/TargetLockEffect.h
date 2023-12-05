#ifndef TARGETLOCKEFFECT_H
#define TARGETLOCKEFFECT_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

#include "Effect.h"
#include <Scripting/Process.h>

class AnimateTargetLockProcess : public mrs::Process
{
public:
    AnimateTargetLockProcess(mrs::Entity target_lock_sprite, float duration);
    virtual ~AnimateTargetLockProcess();

    virtual void OnUpdate(float dt) override;
private:
    mrs::Entity _target_lock_sprite = {};

    float _start_scale = 0.0f;
    float _start_rotation = 0.0f;

    float _end_scale = 0.0f;
    float _end_rotation = 0.0f;

    float _speed = 0.0f;

    float _duration = 0.0f;
    float _time_elapsed = 0.0f;

    float blink_period = 0.25f;
    float time_elapsed = 0.0f;
};

class TargetLockEffect : public Effect
{
public:
    virtual void Init() override;
    virtual void OnUpdate(float dt) override;
private:
    mrs::Entity _target = {};
};

#endif