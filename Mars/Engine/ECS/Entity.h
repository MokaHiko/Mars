#ifndef ENTITY_H
#define ENTITY_H

#pragma once

#include <entt/entt.hpp>
namespace mrs {
    class Scene;
    class Entity
    {
    public:
        Entity(entt::entity id, Scene* scene);
        Entity() = default;

         // Adds component <T> to entity if has not and returns component
         template<typename T, typename... Args>
         T& AddComponent(Args&&...args)
         {
             if (HasComponent<T>()) {
                 return GetComponent<T>();
             }

             return _scene->_registry.emplace<T>(_id, std::forward<Args>(args)...);
         }

         // Returns true if entity has component <T>
         template<typename T>
         bool HasComponent()
         {
             return _scene->_registry.any_of<T>(_id);
         }

         // Returns component of type <T>
         template<typename T>
         T& GetComponent()
         {
             if (HasComponent<T>()) {
                 return _scene->_registry.get<T>(_id);
             }

             throw std::runtime_error("Entity has no such component!");
         }

    private:

        // Entity handle
        entt::entity _id = entt::null;
        Scene* _scene;
    };
}

#endif