#ifndef NATIVESCRIPTCOMPONENT_H
#define NATIVESCRIPTCOMPONENT_H

#pragma once

#include "ECS/ScriptableEntity.h"
#include "Core/Application.h"

namespace mrs
{
    struct Script
    {
        // Prevent pointers from being invalidated on delete
        static constexpr auto in_place_delete = true;

        ScriptableEntity *script = nullptr;
        std::string binding = {};

        std::function<ScriptableEntity *()> InstantiateScript = nullptr;
        std::function<void()> DestroyScript = nullptr;

        bool enabled = false;

        void OnBind(Entity e);

        // Bind scriptable entity
        template <typename T>
        void Bind(Entity e)
        {
            binding = typeid(T).name();

            // Check if script has already been registered
            auto instantiation_fn = script_instantiation_bindings.find(binding);
            if(instantiation_fn == script_instantiation_bindings.end())
            {
                Register<T>();
            }

            InstantiateScript = [&]()
            {
                auto& it = script_instantiation_bindings.find(binding);
                assert(it != script_instantiation_bindings.end() && "Script binding not registerd!");

                return static_cast<ScriptableEntity *>((it->second)());
            };

            DestroyScript = [&]()
            {
                delete script;
                script = nullptr;
            };

            OnBind(e); // Check for Immediate Instantiation
        };

        // Binds script to entity given registered script
        void Bind(std::string binding_name, Entity e)
        {
            binding = binding_name;

            // Assert the script has already been registered
            auto instantiation_fn = script_instantiation_bindings.find(binding);
            assert(instantiation_fn != script_instantiation_bindings.end());

            InstantiateScript = [&]()
            {
                return static_cast<ScriptableEntity *>((script_instantiation_bindings.find(binding)->second)());
            };

            DestroyScript = [&]()
            {
                delete script;
                script = nullptr;
            };

            OnBind(e); // Check for Immediate Instantiation
        }

        // Registers script as valid scriptable entity
        template<typename T>
        static void Register()
        {
            if(script_instantiation_bindings.find(typeid(T).name()) != script_instantiation_bindings.end())
            {
                return;
            }

            script_instantiation_bindings[typeid(T).name()] = []()
            {
                return new T();
            };
        }

        // Map between scriptable entity type id and instantiate functions
        static std::unordered_map <std::string, std::function<ScriptableEntity* ()>> script_instantiation_bindings;
    };
}
#endif