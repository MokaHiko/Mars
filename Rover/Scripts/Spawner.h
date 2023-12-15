#ifndef SPAWNER_H
#define SPAWNER_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>
#include <Scripting/Process.h>

#include "UI/UI.h"

namespace mrs
{
    class Unit : public ScriptableEntity
    {
    public:
        virtual void OnStart() override;
        virtual void OnUpdate(float dt) override;
        virtual void OnCollisionEnter2D(Collision& col) override;
    private:
        Ref<DelayProcess> _delay_destroy = nullptr;
    };

    class Spawner : public ScriptableEntity
    {
    public:
        virtual void OnStart() override;
        virtual void OnUpdate(float dt) override;
    private:
        static Ref<Font> _font;
    };
}
#endif