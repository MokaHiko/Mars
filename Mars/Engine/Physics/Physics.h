#ifndef PHYSICS_H
#define PHYSICS_H

#pragma once

#include <Math/Math.h>

#include <box2d/b2_body.h>
#include <box2d/b2_shape.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_fixture.h>

#include "ECS/Entity.h"
#include "Collider.h"

namespace mrs
{
    enum class BodyType
    {
        UNKNOWN = 0, 
        STATIC,
        DYNAMIC,
        KINEMATIC,
    };

    struct RigidBody2D
    {
    public:
        static constexpr auto in_place_delete = true;

        Vector2 GetVelocity() const;

        float GetAngularVelocity() const;

        // Adds force as impulse
        void AddImpulse(const glm::vec2& value);

        void ApplyAngularImpulse(const float w);

        void SetVelocity(const glm::vec2& value);

        void SetType(BodyType type);

        void SetTransform(const glm::vec2& position, const float angle);

        void SetAngularVelocity(const float w);

        void SetFixedRotation(const bool flag);

        bool Initialized() const {return body != nullptr;}
    public:
        bool use_gravity = true;
        float friction = 0.6f;
        float mass = 1.0f;

        b2Body *body = nullptr;
        BodyType type = BodyType::DYNAMIC;

        float start_angular_velocity = 0.0f;
        Vector2 start_velocity = {0,0};
        Vector2 start_impulse = {0,0};
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

     class Physics2D
     {
     public: 
         // Cast a ray into a scene
         static void Raycast(const Ray& ray, float range = 1000.0f, std::function<void(Collision)> fn = nullptr);
     };
};

#endif