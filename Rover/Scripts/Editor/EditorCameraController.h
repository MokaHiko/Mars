#ifndef EDITORCAMERACONTROLLER_H
#define EDITORCAMERACONTROLLER_H

#pragma once

#include "CameraController.h"

namespace mrs
{
    class EditorCameraController : public CameraController
    {
    public:
        virtual void OnUpdate(float dt) override;

        Entity _focused = {};
    private:
        float _mouse_sensitivity = 0.15f;
        float _movement_speed = 40.0f;
        float _distance = 10.0f;

        void HandleKeyBoardInput(float dt);
        void HandleMouseInput(float dt);
    };
}

#endif