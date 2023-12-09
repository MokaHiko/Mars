#ifndef COMPONENTS_H
#define COMPONENTS_H

#pragma once

#include <glm/glm.hpp>

#include "Renderer/RenderPipelineLayers/RenderPipelines/ParticleRenderPipeline/ParticleComponents.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/TerrainRenderPipeline/TerrainComponent.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/RenderableComponents.h"
#include "Physics/Physics.h"
#include "Scripting/NativeScriptComponent.h"

namespace mrs
{
    // Component given to entities that are serializable. By default typically not present in entities created at runtime
    struct Serializer
    {
        bool serialize = false;
        bool destroy_on_stop = true;
    };

    struct Tag
    {
        Tag() = default;
        Tag(const std::string &name)
            : tag(name) {}

        std::string tag = "";
    };

    struct Transform
    {
    public:
        static const int MAX_CHILDREN = 500;

        Vector3 position{0.0f};
        Vector3 rotation{0.0f};
        Vector3 scale{1.0f};

        Matrix4x4 model_matrix{1.0f};

        Vector3 forward = {0, 0, 1};
        Vector3 back = {0, 0, -1};
        Vector3 up = {0, 1, 0};
        Vector3 down = {0, -1, 0};
        Vector3 right = {1, 0, 0};
        Vector3 left = {-1, 0, 0};

        Entity self = {};
        Entity parent = {};
        std::array<Entity, MAX_CHILDREN> children = {};
        uint32_t children_count = 0;
    public:
        void AddChild(Entity e);
        void RemoveChild(Entity e);

        void UpdateModelMatrix();
        Matrix4x4 LocalModelMatrix();

        const Vector3& GlobalPosition() const;
        Transform& Root();
    private:
        friend class SceneGraphLayer;
        bool dirty_flag = false;
    };
}
#endif