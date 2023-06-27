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

        virtual void OnStart() {}
        virtual void OnUpdate(float dt) {}

        virtual void OnCollisionEnter(Entity other) {}

        // Gets reference to attached entity's component 
        template <typename T>
        T &GetComponent()
        {
            return _game_object.GetComponent<T>();
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

        Entity _game_object = {};
    };
}

#endif