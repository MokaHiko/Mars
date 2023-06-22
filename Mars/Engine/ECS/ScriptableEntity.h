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
        ~ScriptableEntity();

        virtual void OnStart() {}
        virtual void OnUpdate(float dt) {}

        virtual void OnCollisionEnter(Entity other) {}

        void QueueDestroy()
        {
            _game_object._scene->QueueDestroy(_game_object);
        }

        template <typename T>
        T &GetComponent()
        {
            return _game_object.GetComponent<T>();
        }

        Entity Insantiate(const std::string &name = "")
        {
            return _game_object._scene->Instantiate(name);
        }

        Entity _game_object = {};
    };
}

#endif