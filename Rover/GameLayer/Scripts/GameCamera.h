#ifndef GAMECAMERA_H
#define GAMECAMERA_H

#pragma once

#include <ECS/Components/Components.h>
#include <ECS/ScriptableEntity.h>

#include <Renderer/Camera.h>
#include <Core/Input.h>

#include <Scripting/ProcessLayer.h>
#include <Core/Log.h>

class GameCamera : public mrs::ScriptableEntity
{
public:
    mrs::Entity _follow = {};
    float _distance = 100.0f;

    mrs::Camera* _camera = nullptr;
    mrs::Transform* _camera_transform = nullptr;

    float _zoom_distance = 50.0f; 
    float _zoom_duration = 0.5f;

    float _mouse_sensitivity = 0.15f;
    float _movement_speed = 40.0f;
public:
    GameCamera();
    ~GameCamera();

    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;
};

#endif