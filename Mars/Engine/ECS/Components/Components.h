#ifndef COMPONENTS_H
#define COMPONENTS_H

#pragma once

#include "Renderer/Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanMaterial.h"

#include <glm/glm.hpp>

#include "ECS/ScriptableEntity.h"

namespace mrs
{
    struct Tag
    {
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

    // Base renderer class component that makes an object appear on the screen. Inherited by all game object renderers
    struct RenderableObject
    {
        RenderableObject(std::shared_ptr<Mesh> mesh_, std::shared_ptr<Material> material_)
            : mesh(mesh_), material(material_) {}

        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Material> material;
    };

    struct DirectionalLight
    {
        float intensity = 0.0f;
    };

    struct PointLight
    {
        float intensity = 0.0f;
    };

    struct Script
    {
        ScriptableEntity *script = nullptr;

        std::function<ScriptableEntity *()> InstantiateScript;
        std::function<void(ScriptableEntity *script)> DestroyScript;

        template <typename T>
        void Bind()
        {
            InstantiateScript = []()
            {
                return static_cast<ScriptableEntity *>(new T());
            };

            DestroyScript = [](ScriptableEntity *script)
            {
                delete script;
                script = nullptr;
            };
        };
    };
}
#endif