#ifndef GAMECAMERA_H
#define GAMECAMERA_H

#pragma once

#include <ECS/Components/Components.h>
#include <ECS/ScriptableEntity.h>

#include <Renderer/Camera.h>
#include <Core/Input.h>

class GameCamera : public mrs::ScriptableEntity
{
public:
    mrs::Entity _follow = {};
    float _distance = 120.0f;

    mrs::Camera* _camera = nullptr;
    mrs::Transform* _camera_transform = nullptr;

    float _mouse_sensitivity = 0.15f;
    float _movement_speed = 40.0f;
public:
    GameCamera();
    ~GameCamera();

    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;
};

#endif