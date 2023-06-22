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
        Physics2DLayer();
        ~Physics2DLayer();

        virtual void OnAttach() override;
        virtual void OnDetatch() override;

        virtual void OnUpdate(float dt) override;
        virtual void OnImGuiRender() override;

        void AddBody(Entity entity);

        void CreateFixture(Entity entity, BodyType type);

    private:
        b2World* _physics_world;
        ContactListener* _contact_listener;

        Scene* _scene;
    };
}

#endif