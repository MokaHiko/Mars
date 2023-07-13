#ifndef ENTITY_H
#define ENTITY_H

#pragma once

#include <entt/entt.hpp>

#include "Core/Log.h"
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
            if (HasComponent<T>())
            {
                return GetComponent<T>();
            }

            return _scene->_registry.emplace<T>(_id, std::forward<Args>(args)...);
        }

        // Removes component <T> from entity returns true if succeseful 
        template <typename T, typename... Args>
        bool RemoveComponent(Args &&...args)
        {
            if (HasComponent<T>())
            {
                return false;
            }

            // TODO: Event manager on remove compoenent/entity
            _scene->_registry.remove<T>(_id);
            return true;
        }

        // Returns true if entity has component <T>
        template <typename T>
        bool HasComponent()
        {
            return _scene->_registry.any_of<T>(_id);
        }

        // Returns component of type <T>
        template <typename T>
        T &GetComponent()
        {
            if (HasComponent<T>())
            {
                return _scene->_registry.get<T>(_id);
            }

            MRS_ERROR("Entity has no such component!");
            throw std::runtime_error("Entity has no such component!");
        }

        // Returns entity handle as uint32_t
        const uint32_t Id() const { return static_cast<uint32_t>(_id); }

        bool operator==(const Entity &other) const { return _id == other._id; }
        operator bool() const { return _id != entt::null; }
        operator entt::entity() const { return _id; }

    private:
        friend class Scene;
        friend class ScriptableEntity;

        // Entity handle
        entt::entity _id = entt::null;
        Scene *_scene;
    };
}

#endif