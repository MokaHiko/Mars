#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#pragma once

#include "ECS/ScriptableEntity.h"

#include "Renderer/Camera.h"
#include "Core/Input.h"

#include "SDL.h"
#include "ECS/Components/Components.h"

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

        virtual void OnStart() override
        {
            // Get handles
            _camera = &GetComponent<Camera>();
            _camera_transform = &GetComponent<Transform>();
        };

        virtual void OnUpdate(float dt) override
        {
            if (_camera->_active)
            {
                HandleKeyBoardInput(dt);
                HandleMouseInput(dt);
			    _camera->UpdateViewProj();
            }
        };

    private:
        void HandleKeyBoardInput(float dt)
        {
            Transform &camera_transform = *_camera_transform;

            if (Input::IsKeyPressed(SDLK_f))
            {
                if (_focused)
                {
                    camera_transform = _focused.GetComponent<Transform>();
                    camera_transform.position.z -= _distance;
                }
            }

            if (Input::IsKeyPressed(SDLK_SPACE))
            {
                camera_transform.position += _camera->Up() * _movement_speed * dt;
            }

            else if (Input::IsKeyPressed(SDLK_q))
            {
                camera_transform.position -= _camera->Up() * _movement_speed * dt;
            }

            if (Input::IsKeyPressed(SDLK_w))
            {
                camera_transform.position += _camera->Front() * _movement_speed * dt;
            }
            else if (Input::IsKeyPressed(SDLK_s))
            {
                camera_transform.position -= _camera->Front() * _movement_speed * dt;
            }

            if (Input::IsKeyPressed(SDLK_d))
            {
                camera_transform.position += _camera->Right() * _movement_speed * dt;
            }
            else if (Input::IsKeyPressed(SDLK_a))
            {
                camera_transform.position -= _camera->Right() * _movement_speed * dt;
            }

            _camera->GetPosition() = _camera_transform->position;
        }
        void HandleMouseInput(float dt)
        {
            if (!Input::IsMouseButtonPressed(SDL_BUTTON_RIGHT))
            {
                return;
            }

            if (float x = Input::GetAxis('x'))
            {
                _camera->GetYaw() += (x * _mouse_sensitivity);
            }

            if (float y = Input::GetAxis('y'))
            {
                float pitch_change = _camera->GetPitch();
                pitch_change -= (y * _mouse_sensitivity);

                _camera->GetPitch() = pitch_change;

                if (pitch_change > 89.0f)
                {
                    _camera->GetPitch() = 89.0f;
                }

                if (pitch_change < -89.0f)
                {
                    _camera->GetPitch() = -89.0f;
                }
            }
        }
    };

}

#endif