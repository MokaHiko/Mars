#ifndef SCENEGRAPHLAYER_H
#define SCENEGRAPHLAYER_H

#pragma once

#include "Core/Layer.h"
#include "ECS/Scene.h"
namespace mrs
{
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
        Scene* _scene = nullptr;
    };
}

#endif