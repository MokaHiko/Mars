#ifndef PHYSICS_H
#define PHYSICS_H

#pragma once

#include<glm/glm.hpp>

#include <box2d/b2_body.h>
#include <box2d/b2_shape.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_fixture.h>

#include "ECS/Entity.h"
#include "Core/Application.h"
#include "Collider.h"

namespace mrs
{
    enum class BodyType
    {
        UNKNOWN = 0, 
        STATIC,
        DYNAMIC
    };

    struct RigidBody2D
    {
    public:
        static constexpr auto in_place_delete = true;

        // Adds force as impulse
        void AddImpulse(const glm::vec2& value)
        {
            body->ApplyLinearImpulse({value.x, value.y}, body->GetWorldCenter(), true);
        }

        void SetMass(float mass)
        {
            b2MassData mass_data = {};
            mass_data.mass = mass;
            body->SetMassData(&mass_data);
        }

        // Sets instantaneous velocity 
        void SetVelocity(const glm::vec2& value)
        {
            body->SetLinearVelocity({value.x, value.y});
        }

        // Sets instantaneous angular velocity in radians/second
        void SetAngularVelocity(const float w)
        {
            body->SetAngularVelocity(w);
        }

        void ApplyAngularImpulse(const float w)
        {
            body->ApplyAngularImpulse(w, true);
        }

        void SetGravityScale(const float value = 0.0f)
        {
            body->SetGravityScale(value);
        };

        void SetFixedRotation(const bool flag)
        {
            body->SetFixedRotation(flag);
        };

    public:
        bool use_gravity = true;
        float friction = 0.6f;
        b2Body *body = nullptr;
        BodyType type = BodyType::DYNAMIC;
    };

    struct BoxCollider2D
    {
        static constexpr auto in_place_delete = true;

        float width = 1.0f;
        float height = 1.0f;
    };

    struct MeshCollider
    {
        // TODO: Make pointer safe
        ColliderType type;
        Ref<Collider> collider = nullptr;
    };

    struct Ray
    {
        glm::vec3 origin;
        glm::vec3 direction;
    };

    struct Collision
    {
        Entity entity = {};
        CollisionPoints collision_points = {};
    };

    class Physics
    {
    public: 
        // Cast a ray into a scene
        static const Collision Raycast(Scene* scene, const Ray& ray, float range = 1000.0f);
    };
};

#endif