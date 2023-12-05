#ifndef SCENE_H
#define SCENE_H

#pragma once

#include <entt/entt.hpp>

#include <glm/glm.hpp>
#include <ToolBox/SignalToolBox.h>

#include "Core/Log.h"
#include "Core/Memory.h"

namespace mrs
{
    class Entity;

    // Wraps enTT ECS functionality 
    class Scene
    {
    public:
        Scene& operator=(const Scene& other) = delete;
        Scene(const Scene& other) = delete;

        Scene();
        ~Scene();

        void Serialization(bool serializing) { _serializing = serializing; }

        Entity Root();

        entt::registry* Registry()
        {
            return &_registry;
        }

        template<typename T>
        Entity FindEntityWithComponent()
        {
            for (entt::entity e : _registry.view<T>())
            {
                return Entity{ e, this };
            }

            return {};
        }

        template <typename T, typename... Args>
        T& AddComponent(entt::entity id, Args &&...args)
        {
            if (HasComponent<T>(id))
            {
                return GetComponent<T>(id);
            }

            return _registry.emplace<T>(id, std::forward<Args>(args)...);
        }

        template <typename ScriptType>
        ScriptType& AddScript(entt::entity id)
        {
            if (HasComponent<Script>(id))
            {
                if (ScriptType* script = dynamic_cast<ScriptType*>(GetComponent<Script>(id).script))
                {
                    return *script;
                }
                else
                {
                    MRS_ERROR("Script Type does not exist on this entity!");
                }
            }

            auto& script_component = _registry.emplace<Script>(id);
            script_component.Bind<ScriptType>({id, this});

            ScriptType* script = dynamic_cast<ScriptType*>(script_component.script);
            return *script;
        }

        template <typename T, typename... Args>
        bool RemoveComponent(entt::entity id, Args &&...args)
        {
            if (HasComponent<T>(id))
            {
                return false;
            }

            _registry.remove<T>(id);
            return true;
        }

        template <typename T>
        bool HasComponent(entt::entity id)
        {
            return _registry.any_of<T>(id);
        }

        template <typename T>
        T& GetComponent(entt::entity id)
        {
            if (HasComponent<T>(id))
            {
                return _registry.get<T>(id);
            }

            MRS_ERROR("Entity has no such component!");
            throw std::runtime_error("Entity has no such component!");
        }

        template <typename T>
        T* GetScript(entt::entity id)
        { 
            if (HasComponent<Script>(id))
            {
		        return dynamic_cast<T*>(_registry.get<Script>(id).script);
            }

            return nullptr;
        }

        // Create and returns entity
        Entity Instantiate(const std::string& name = "", const glm::vec3& position = {}, bool* serialize = nullptr);

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
        entt::entity _root;

        // Entity ids free to be reused
        std::vector<entt::entity> _free_queue;

        // Queue of entities to destroy
        std::list<Entity> _destruction_queue;

        // Serialization mode
        bool _serializing = false;
    };
}
#endif