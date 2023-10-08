#ifndef NATIVESCRIPTINGLAYER_H
#define NATIVESCRIPTINGLAYER_H

#pragma once

#include "Core/Layer.h"
#include "ECS/Components/Components.h"

namespace mrs
{
    class NativeScriptingLayer : public Layer
    {
    public:
        virtual void OnAttach();
        virtual void OnEnable() override;
        virtual void OnDisable() override;

        virtual void OnUpdate(float dt) override;
        virtual void OnEvent(Event &event) override;

        void DisableScripts(Entity except = {});
        void EnableScripts(Entity except = {});

        void OnScriptDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity);
    private:
        Scene *_scene = nullptr;
    };
}

#endif