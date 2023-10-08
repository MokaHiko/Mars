#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#pragma once

#include "ECS/ScriptableEntity.h"

#include "Renderer/Camera.h"
#include "Core/Input.h"

#include "SDL.h"
#include "ECS/Components/Components.h"
#include "Physics/Physics.h"

namespace mrs
{
    class CameraController : public ScriptableEntity
    {
    public:
        Entity _focused = {};
        float _distance = 10.0f;

        Camera *_camera = nullptr;
        Transform *_camera_transform = nullptr;

        float _mouse_sensitivity = 0.15f;
        float _movement_speed = 40.0f;
    public:
        CameraController() = default;
        ~CameraController() {};

        virtual void OnStart() override;
        virtual void OnUpdate(float dt) override;

        const Ray ScreenPointToRay(const glm::vec2& point) const;

    private:
        void HandleKeyBoardInput(float dt);
        void HandleMouseInput(float dt);
    };
}

#endif