#ifndef PHYSICS2DLAYER_H
#define PHYSICS2DLAYER_H

#pragma once

#include "Core/Layer.h"

#include <box2d/b2_contact.h>
#include <box2d/b2_world.h>

#include "ECS/Scene.h"
#include "Physics.h"

namespace mrs
{
    class ContactListener : public b2ContactListener
    {
    public:
        ContactListener(Scene *scene) : _scene(scene) {}

        // Called when two fixtures begin to touch
        virtual void BeginContact(b2Contact *contact) override;

        // Called when two fixtures cease to touch
        virtual void EndContact(b2Contact *contact) override;

    private:
        Scene *_scene;
    };

    class Physics2DLayer : public Layer
    {
    public:
        virtual void OnAttach() override;
        virtual void OnDetatch() override;

        virtual void OnUpdate(float dt) override;
        virtual void OnImGuiRender() override;

        virtual void OnEnable() override;
        virtual void OnDisable() override;

        void AddBody(Entity entity);
        void CreateFixture(Entity entity, BodyType type);

        void OnRigidBody2DCreated(entt::basic_registry<entt::entity>&, entt::entity entity);
        void OnRigidBody2DDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity);
    private:
        void InitWorld();
        void ShutdownWorld();

        void FlushQueues(); // Operates and Clears physics request queues
    private:
        std::vector<Entity> _construction_queue;
        std::vector<b2Body*> _destruction_queue;

        b2World* _physics_world = nullptr;
        ContactListener* _contact_listener = nullptr;

        Scene* _scene;
    };
}

#endif