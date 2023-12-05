#ifndef UILAYER_H
#define UILAYER_H

#pragma once

#include "Core/Layer.h"
#include "ECS/Scene.h"

namespace mrs
{
    class UILayer : public Layer
    {
    public:
        UILayer();
        ~UILayer();

        // virtual void OnAttach() override;
        // virtual void OnDetatch() override;

        virtual void OnEnable() override;
        virtual void OnDisable() override;

        virtual void OnUpdate(float dt) override;
    private:
        Scene* _scene = nullptr;
    };
}

#endif