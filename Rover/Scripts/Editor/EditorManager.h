#ifndef EDITORMANAGER_H
#define EDITORMANAGER_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>
#include <Renderer/Renderer.h>
#include <Core/Application.h>
#include <Core/Log.h>

#include "CameraController.h"
namespace mrs
{
    class EditorManager : public ScriptableEntity
    {
    public:
        CameraController* _camera_controller = nullptr;
        Entity _camera = {};
    public:
        EditorManager() {};
        ~EditorManager() {};

        virtual void OnCreate() override
        {
            MRS_TRACE("Initializing editor manager!");
            InitEditorResources();
        }

        virtual void OnStart() override
        {
            _camera_controller = dynamic_cast<CameraController*>(_camera.GetComponent<Script>().script);
        }

        virtual void OnUpdate(float dt) override 
        {
        }

        virtual void OnCollisionEnter(Entity other) override 
        {
        }

    private:
        void InitEditorResources()
        {
			// Scene
			Scene *scene = Application::GetInstance().GetScene();

			// Camera
			auto window = Application::GetInstance().GetWindow();
			_camera = scene->Instantiate("Editor Camera");

			_camera.AddComponent<Camera>(CameraType::Perspective, window->GetWidth(), window->GetHeight());
            _camera.AddComponent<Script>().Bind<CameraController>();
			_camera.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, 20.0f);
        };
    };
}

#endif