#ifndef SCENE_H
#define SCENE_H

#pragma once

#include <entt/entt.hpp>

#include "Components/Components.h"
#include "Entity.h"

namespace mrs {
    // Main wrapper of wraps enTT ECS functionality 
    class Scene
    {
    public:
        Scene& operator=(const Scene& other) = delete;
        Scene(const Scene& other) = delete;

        Scene() {};
        ~Scene() {};

        entt::registry* Registry() {
            return &_registry;
        }
        
        // Create and returns entity
        Entity Instantiate(const std::string& name = "", const glm::vec3& position = {}) {
            Entity e = Entity(_registry.create(), this);

            e.AddComponent<Tag>(name);
            e.AddComponent<Transform>();

            return e;
        }

    private:
        friend class Entity;
        entt::registry _registry;
    };

}

#endif