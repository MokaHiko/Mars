#ifndef SCRAP_H
#define SCRAP_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

#include <Toolbox/RandomToolBox.h>

enum class ResourceType
{
    UKNOWN,
    SCRAP_METAL,
    CREDIT
};

struct Resource
{
    uint32_t amount;
    ResourceType type;
};

class Scrap : public mrs::ScriptableEntity
{
public:
    virtual void OnStart() override;
    virtual void OnCollisionEnter2D(mrs::Entity other) override;

    void Fade();
private:
    const float _life_span = 60.0f;
    float _explosion_magnitude = 1.0f;

    static tbx::PRNGenerator<float> _random_angle;
    static tbx::PRNGenerator<float> _random;
};

#endif