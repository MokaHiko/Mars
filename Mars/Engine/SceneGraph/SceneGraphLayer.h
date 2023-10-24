#ifndef SCENEGRAPHLAYER_H
#define SCENEGRAPHLAYER_H

#pragma once

#include "Core/Layer.h"
#include "ECS/Scene.h"
namespace mrs
{
    struct Transform;

    // Updates transforms and relative transform components of the scene hierarchy
    class SceneGraphLayer : public Layer
    {
    public:
        SceneGraphLayer();
        virtual ~SceneGraphLayer();

        virtual void OnAttach() override;
        virtual void OnDetatch() override;

        virtual void OnEnable()override;
        virtual void OnDisable()override;

        virtual void OnUpdate(float dt) override;
    private:
        void OnTransformCreated(entt::basic_registry<entt::entity>&, entt::entity entity);
        void OnTransformDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity);

        // Models are hierarchies of meshes
        void OnModelRendererCreated(entt::basic_registry<entt::entity>&, entt::entity entity);
        void RecursiveUpdate(Transform& node);

        Scene* _scene = nullptr;
    };
}

#endif