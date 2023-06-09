#ifndef SCRIPTABLEENTITY_H
#define SCRIPTABLEENTITY_H

#pragma once

#include "Entity.h"

namespace mrs
{
    class ScriptableEntity
    {
    public:
        ScriptableEntity();
        virtual ~ScriptableEntity();

        virtual void OnCreate() {}
        virtual void OnStart() {}
        virtual void OnUpdate(float dt) {}

        virtual void OnCollisionEnter(Entity other) {}

        // Gets reference to attached entity's component 
        template <typename T>
        T &GetComponent()
        {
            return _game_object.GetComponent<T>();
        }

        // Finds first entity with the component type
        template<typename T>
        Entity FindEntityWithComponent()
        {
            return _game_object._scene->FindEntityWithComponent<T>();
        }

        // Insantiates new entity
        Entity Insantiate(const std::string &name = "")
        {
            return _game_object._scene->Instantiate(name);
        }

        // Queues attached entity for destruction
        void QueueDestroy()
        {
            _game_object._scene->QueueDestroy(_game_object);
        }

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

        Entity _game_object = {};
    };
}

#endif