#ifndef SPAWNER_H
#define SPAWNER_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

namespace mrs
{
    class Unit : public ScriptableEntity
    {
    public:
        virtual void OnUpdate(float dt) override;
    private:
        int dir = 1;
        float time_elapsed = 0.0f;
    };

    class Spawner : public ScriptableEntity
    {
    public:
        virtual void OnStart() override;
        virtual void OnUpdate(float dt) override;
    private:

    };
}
#endif