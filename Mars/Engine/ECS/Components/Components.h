#ifndef COMPONENTS_H
#define COMPONENTS_H

#pragma once

#include <glm/glm.hpp>

#include "Renderer/RenderPipelineLayers/RenderPipelines/ParticleRenderPipeline/ParticleComponents.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/RenderableComponents.h"
#include "Physics/Physics.h"
#include "Scripting/NativeScriptComponent.h"

namespace mrs
{
    struct Tag
    {
        Tag() = default;
        Tag(const std::string &name)
            : tag(name) {}
        std::string tag = "";
    };

    struct Transform
    {
        glm::vec3 position{0.0f};
        glm::vec3 rotation{0.0f};
        glm::vec3 scale{1.0f};
    };
}
#endif