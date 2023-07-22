#ifndef RENDERABLECOMPONENTS_H
#define RENDERABLECOMPONENTS_H

#pragma once

#include "ECS/Entity.h"

#include "Renderer/Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanMaterial.h"
#include "Renderer/Camera.h"

namespace mrs
{
    // Base renderer class component that makes an object appear on the screen. Inherited by all game object renderers
    struct RenderableObject
    {
        RenderableObject()
        {
            mesh = Mesh::Get("cube");
            material = Material::Get("default_material");
        };

        RenderableObject(std::shared_ptr<Mesh> mesh_, std::shared_ptr<Material> material_)
            : mesh(mesh_), material(material_) {}

        const std::shared_ptr<Mesh>& GetMesh() const
        {
            return mesh;
        }

        void SetMesh(std::shared_ptr<Mesh> new_mesh)
        {
            mesh = new_mesh;
        }

        const std::shared_ptr<Material> GetMaterial() const 
        {
            return material;
        }
        
        void SetMaterial(std::shared_ptr<Material> new_material);

    private:
        std::shared_ptr<Mesh> mesh = nullptr;
        std::shared_ptr<Material> material = nullptr;
    };

    struct DirectionalLight
    {
        float intensity = 0.0f;
    };

    struct PointLight
    {
        float intensity = 0.0f;
    };
}

#endif