#ifndef RENDERABLECOMPONENTS_H
#define RENDERABLECOMPONENTS_H

#pragma once

#include "ECS/Entity.h"

#include "Renderer/Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanMaterial.h"
#include "Renderer/Camera.h"
#include "Renderer/Model.h"
#include "Renderer/Textures/Sprite.h"
#include "Math/Math.h"

namespace mrs
{
    // Base renderer class component that makes an object appear on the screen. Inherited by all rendererable types.
    struct Renderable
    {
        Renderable()
        {
            material = Material::Get("default");
        }

        Renderable(Ref<Material> material_)
        {
            material = material_;
        }

        Ref<Material> material = nullptr;
        bool enabled = true;
    };

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

    struct ModelRenderer 
    {
        ModelRenderer(Ref<Model> m)
        {
            model = m;
        }

        Ref<Model> model;
    };

    struct SpriteRenderer
    {
        Ref<Sprite> sprite = nullptr; 
        int sprite_index = 0;
        Vector4 color {1.0f};
        uint32_t sorting_layer = 0;
    };

    struct SpriteAnimator
    {
        float fps = 12.0f;
        float time_elapsed = 0.0f;

        void Play(int frame = -1);
        void Stop();

        bool playing = true;
    };

    struct PointLight
    {
        float intensity = 0.0f;
    };
}

#endif