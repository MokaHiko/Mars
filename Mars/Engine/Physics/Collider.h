#ifndef COLLIDER_H
#define COLLIDER_H

#pragma once

#include <glm/glm.hpp>

namespace mrs
{
    enum class ColliderType : uint8_t
    {
        Uknown,
        SphereCollider,
        PlaneCollider,
    };

    struct CollisionPoints
    {
        glm::vec3 a; // Furthest point of collider a into b
        glm::vec3 b; // Furthest point of collider b into a
        glm::vec3 normal; // b - a normalized
        float depth; // magnitude of b - a

        bool did_collide;
    };

    struct Transform;
    struct SphereCollider;
    struct PlaneCollider;
    struct Ray;
    class Collider
    {
    public:
        Collider();
        virtual ~Collider();

        virtual CollisionPoints TestRayCollision(
            const Transform* transform,
            const Ray* ray
        ) const = 0;

        virtual CollisionPoints TestCollision(
            const Transform* transform, 
            const Collider* other,   
            const Transform* other_transform
        ) const = 0;

        virtual CollisionPoints TestCollision(
            const Transform* transform,
            const SphereCollider* other,
            const Transform* other_transform
        ) const = 0;

        virtual CollisionPoints TestCollision(
            const Transform* transform,
            const PlaneCollider* other,
            const Transform* other_transform
        ) const = 0;
    private:
    };

    struct SphereCollider : public Collider
    {
    public:
        SphereCollider(){}
        virtual ~SphereCollider(){};

        float radius = 1.0f;

        virtual CollisionPoints TestRayCollision(
            const Transform* transform,
            const Ray* ray
        ) const override;

        virtual CollisionPoints TestCollision(
            const Transform* transform,
            const Collider* other,
            const Transform* other_transform
        ) const override;

        virtual CollisionPoints TestCollision(
            const Transform* transform,
            const SphereCollider* other,
            const Transform* other_transform
        ) const override;

        virtual CollisionPoints TestCollision(
            const Transform* transform,
            const PlaneCollider* other,
            const Transform* other_transform
        ) const override;
    };

    struct PlaneCollider : public Collider
    {
    public:
        PlaneCollider(){}
        virtual ~PlaneCollider(){};

        float width = 1.0f;
        float height = 1.0f;
        glm::vec3 normal = { 0.0f, 1.0f, 0.0f };

        virtual CollisionPoints TestRayCollision(
            const Transform* transfarm,
            const Ray* ray
        ) const override;

        virtual CollisionPoints TestCollision(
            const Transform* transform,
            const Collider* other,
            const Transform* other_transform
        ) const override;

        virtual CollisionPoints TestCollision(
            const Transform* transform,
            const SphereCollider* other,
            const Transform* other_transform
        ) const override;

        virtual CollisionPoints TestCollision(
            const Transform* transform,
            const PlaneCollider* other,
            const Transform* other_transform
        ) const override;
    };
}

#endif