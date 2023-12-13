#ifndef ENTITY_H
#define ENTITY_H

#pragma once

#include <entt/entt.hpp>
#include "Scene.h"

namespace mrs
{
    class Entity
    {
    public:
        Entity(entt::entity id, Scene *scene);
        Entity() = default;

        // Adds component <T> to entity if has not and returns component
        template <typename T, typename... Args>
        T &AddComponent(Args &&...args)
        {
            return _scene->AddComponent<T>(_id, args...);
        }

        // Removes component <T> from entity returns true if succeseful 
        template <typename T, typename... Args>
        bool RemoveComponent(Args &&...args)
        {
            return _scene->RemoveComponent<T>(_id, args...);
        }

        // Returns true if entity has component <T>
        template <typename T>
        bool HasComponent() const
        {
            return _scene->HasComponent<T>(_id);
        }

        // Returns component of type <T>
        template <typename T>
        T &GetComponent()
        {
           return _scene->GetComponent<T>(_id);
        }

        // Adds component <T> to entity if has not and returns component
        template <typename ScriptType>
        ScriptType &AddScript()
        {
            return _scene->AddScript<ScriptType>(_id);
        }

        // Returns component of type <T>
        template <typename T>
        T* GetScript()
        {
            return _scene->GetScript<T>(_id);
        }

        bool IsAlive() const;

        // Returns entity handle
        const uint32_t Id() const { return static_cast<uint32_t>(_id); }

        bool operator==(const Entity &other) const { return _id == other._id; }
        bool operator!=(const Entity &other) const { return _id != other._id; }

        friend std::ostream& operator<<(std::ostream& stream, Entity& e);

        operator bool() const { return _id != entt::null; }
        operator entt::entity() const { return _id; }

    private:
        friend class Scene;
        friend class ScriptableEntity;

        entt::entity _id = entt::null;
        Scene *_scene = nullptr;
    };
}

#endif