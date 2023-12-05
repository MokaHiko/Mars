#ifndef SCRIPTABLEENTITY_H
#define SCRIPTABLEENTITY_H

#pragma once

#include "Entity.h"
#include "Math/Math.h"

namespace mrs
{
    class Process;
    class ScriptableEntity
    {
    public:
        ScriptableEntity();
        virtual ~ScriptableEntity();

        virtual void OnCreate() {}
        virtual void OnStart() {}
        virtual void OnUpdate(float dt) {}

        virtual void OnCollisionEnter2D(Entity other) {}

        // Gets reference to attached entity's component 
        template <typename T>
        T &GetComponent()
        {
            return _game_object.GetComponent<T>();
        }

        // Gets reference to attached entity's component 
        template <typename T, typename... Args>
        T &AddComponent(Args &&...args)
        {
            return _game_object.AddComponent<T>(args...);
        }

        // Finds first entity with the component type
        template<typename T>
        Entity FindEntityWithComponent()
        {
            return _game_object._scene->FindEntityWithComponent<T>();
        }

        // Insantiates new entity
        Entity Instantiate(const std::string &name = "", Vector3 position = {0,0,0});

        // Queues attached entity for destruction
        void QueueDestroy();
        
        template<typename ScriptType>
        Entity FindEntityWithScript()
        {
            for(auto e: _game_object._scene->Registry()->view<Transform, Script>())
            {
                Entity entity{e, _game_object._scene};
                auto& script_component = entity.GetComponent<Script>();

                if(script_component.binding == typeid(ScriptType).name())
                {
                    return entity;
                }
            }

            return {};
        }

        template<typename ScriptType>
        std::vector<Entity> FindEntitiesWithScript()
        {
            std::vector<Entity> entities;

            for(auto e: _game_object._scene->Registry()->view<Transform, Script>())
            {
                Entity entity{e, _game_object._scene};
                auto& script_component = entity.GetComponent<Script>();

                if(script_component.binding == typeid(ScriptType).name())
                {
                    entities.push_back(entity);
                }
            }

            return entities;
        }

        void StartProcess(Ref<Process> process);

        void Reset();

        Entity _game_object = {};
    private:
        friend class NativeScriptingLayer;
        bool _on_start_called = false;
        bool _on_create_called = false;

        bool _queued_destroy = false;

        std::vector<Ref<Process>> _processes;
    };
}

#endif