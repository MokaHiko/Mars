#ifndef SCENE_H
#define SCENE_H

#pragma once

#include <entt/entt.hpp>

#include <glm/glm.hpp>

namespace mrs
{
    class Entity;

    // Wraps enTT ECS functionality 
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
        Entity Instantiate(const std::string& name = "", const glm::vec3& position = {});

        // Queues entity for destruction at end of frame
        void QueueDestroy(Entity entity);

    private:
        void Destroy(Entity entity);

        friend class Entity;
        entt::registry _registry;

        // Queue of entities to destroy
        std::vector<Entity> _destruction_queue;
    };

}

#endif