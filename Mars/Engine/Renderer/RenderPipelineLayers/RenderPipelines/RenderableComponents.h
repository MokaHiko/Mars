#ifndef RENDERABLECOMPONENTS_H
#define RENDERABLECOMPONENTS_H

#pragma once

#include "ECS/Entity.h"

#include "Renderer/Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanMaterial.h"
#include "Renderer/Camera.h"
#include "Renderer/Model.h"

namespace mrs
{
    struct ModelRenderer
    {
        ModelRenderer(Ref<Model> m)
        {
            model = m;
        }

        Ref<Model> model;
    };

    // Base renderer class component that makes an object appear on the screen. Inherited by all rendererable types.
    struct MeshRenderer
    {
        MeshRenderer()
        {
            mesh = Mesh::Get("cube");
            material = Material::Get("default");
        };

        MeshRenderer(Ref<Mesh> mesh_, Ref<Material> material_)
            : mesh(mesh_), material(material_) {}

        const Ref<Mesh>& GetMesh() const
        {
            return mesh;
        }

        void SetMesh(Ref<Mesh> new_mesh)
        {
            mesh = new_mesh;
        }

        const Ref<Material> GetMaterial() const 
        {
            return material;
        }
        
        void SetMaterial(Ref<Material> new_material);

    private:
        Ref<Mesh> mesh = nullptr;
        Ref<Material> material = nullptr;
    };

    struct PointLight
    {
        float intensity = 0.0f;
    };
}

#endif