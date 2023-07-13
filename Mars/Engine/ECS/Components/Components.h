#ifndef COMPONENTS_H
#define COMPONENTS_H

#pragma once

#include "Renderer/Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanMaterial.h"

#include <glm/glm.hpp>

#include "ECS/ScriptableEntity.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/ParticleRenderPipeline/ParticleComponents.h"
#include "Physics/Physics.h"
#include "Renderer/Camera.h"

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

    struct Script
    {
        ScriptableEntity *script = nullptr;
        std::string binding = {};

        std::function<ScriptableEntity *()> InstantiateScript = nullptr;
        std::function<void()> DestroyScript = nullptr;

        bool enabled = true;

        // Bind scriptable entity
        template <typename T>
        void Bind()
        {
            binding = typeid(T).name();

            // Check if script has already been registered
            auto instantiation_fn = script_instantion_bindings.find(binding);
            if(instantiation_fn == script_instantion_bindings.end())
            {
                Register<T>();
            }

            InstantiateScript = [&]()
            {
                auto& it = script_instantion_bindings.find(binding);
                assert(it != script_instantion_bindings.end() && "Script binding not registerd!");

                return static_cast<ScriptableEntity *>((it->second)());
            };

            DestroyScript = [&]()
            {
                delete script;
                script = nullptr;
            };
        };

        // Binds script to entity given registered script
        void Bind(std::string binding_name)
        {
            binding = binding_name;

            // Assert the script has already been registered
            auto instantiation_fn = script_instantion_bindings.find(binding);
            assert(instantiation_fn != script_instantion_bindings.end());

            InstantiateScript = [&]()
            {
                return static_cast<ScriptableEntity *>((script_instantion_bindings.find(binding)->second)());
            };

            DestroyScript = [&]()
            {
                delete script;
                script = nullptr;
            };
        }

        // Registers script as valid scriptable entity
        template<typename T>
        static void Register()
        {
            script_instantion_bindings[typeid(T).name()] = []()
            {
                return new T();
            };
        }

        // Map between scriptable entity type id and instantiate functions
        static std::unordered_map <std::string, std::function<ScriptableEntity* ()>> script_instantion_bindings;
    };
}
#endif