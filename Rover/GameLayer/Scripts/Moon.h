#ifndef MOON_H
#define MOON_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>
#include <Math/Math.h>

struct MoonProperties
{
    mrs::Entity _planet;
    float a, b = 1;
};

class Moon : public mrs::ScriptableEntity
{
public:
    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;
private:
    MoonProperties _props;
    float theta = 0;
};

#endif