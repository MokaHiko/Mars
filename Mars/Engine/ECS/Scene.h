#ifndef SCENE_H
#define SCENE_H

#pragma once

#include <entt/entt.hpp>

#include <glm/glm.hpp>
#include <ToolBox/SignalToolBox.h>

namespace mrs
{
    class Entity;

    // Wraps enTT ECS functionality 
    class Scene
    {
    public:
        Scene &operator=(const Scene &other) = delete;
        Scene(const Scene &other) = delete;

        Scene();
        ~Scene();

		void Serialization(bool serializing) {_serializing = serializing;}

        entt::registry *Registry()
        {
            return &_registry;
        }

        template<typename T>
        Entity FindEntityWithComponent()
        {
            for(auto e : _registry->view<T>())
            {
                return {e, this};
            }

            return {};
        }

        // Create and returns entity
        Entity Instantiate(const std::string &name = "", const glm::vec3 &position = {}, bool* serialize = nullptr);

        // Queues entity for destruction at end of frame
        void QueueDestroy(Entity entity);

        // Clears destroys all entities and components in destruction queue
        void FlushDestructionQueue();

        // Immediately destroys entity
        void Destroy(Entity entity);

        // ECS signals
        tbx::Signal<Entity> _entity_destroyed;
        tbx::Signal<Entity> _entity_created;
    private:
        friend class Entity;
        entt::registry _registry;

        // Entity ids free to be reused
        std::vector<Entity> _free_queue;

        // Queue of entities to destroy
        std::vector<Entity> _destruction_queue;

        // Serialization mode
		bool _serializing = false;
    };

}

#endif