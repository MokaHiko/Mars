#ifndef EDITORMANAGER_H
#define EDITORMANAGER_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>
#include <Renderer/Renderer.h>
#include <Core/Application.h>
#include <Core/Log.h>

#include "EditorCameraController.h"
namespace mrs
{
    class EditorManager : public ScriptableEntity
    {
    public:
        EditorCameraController* _camera_controller = nullptr;
        Entity _camera = {};
    public:
        virtual void OnCreate() override;

        virtual void OnStart() override;
    private:
        void InitEditorResources();
    };
}

#endif