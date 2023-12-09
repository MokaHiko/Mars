#ifndef GAMECAMERA_H
#define GAMECAMERA_H

#pragma once

#include <Scripts/Editor/CameraController.h>

class GameCamera : public mrs::CameraController
{
public:
    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;
private:
    mrs::Entity _follow = {};
    float _distance = 100.0f;

    float _zoom_distance = 50.0f; 
    float _zoom_duration = 0.5f;

    float _mouse_sensitivity = 0.15f;
    float _movement_speed = 40.0f;
};

#endif