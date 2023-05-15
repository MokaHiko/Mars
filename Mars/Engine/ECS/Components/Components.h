#ifndef COMPONENTS_H
#define COMPONENTS_H

#pragma once

#include "Renderer/Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanMaterial.h"

#include <glm/glm.hpp>

namespace mrs {
    // [Component]
    struct Tag
    {
        std::string tag = "";
    };

    // [Component]
    struct Transform
    {
        glm::vec3 position{ 0.0f };
        glm::vec3 rotation{ 0.0f };
        glm::vec3 scale{ 1.0f };
    };

    // [Component]
    struct RenderableObject
    {
        RenderableObject(std::shared_ptr<Mesh> mesh_, std::shared_ptr<Material> material_)
            :mesh(mesh_), material(material_) {}

        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Material> material;
    };

    // [Component]
    struct DirectionalLight
    {
        float intensity = 0.0f;
    };
}

#endif