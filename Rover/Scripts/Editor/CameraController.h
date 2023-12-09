#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

#include "Renderer/Camera.h"

namespace mrs
{
    class CameraController : public ScriptableEntity
    {
    public:
        Camera *_camera = nullptr;
        Transform *_camera_transform = nullptr;
    public:
        virtual void OnCreate() final;
        const Ray ScreenPointToRay(const glm::vec2& point) const;
    };
}

#endif