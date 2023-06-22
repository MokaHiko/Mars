#ifndef PHYSICS_H
#define PHYSICS_H

#pragma once

#include<glm/glm.hpp>

#include <box2d/b2_body.h>
#include <box2d/b2_shape.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_fixture.h>

namespace mrs
{
    enum class BodyType
    {
        UNKNOWN, 
        STATIC,
        DYNAMIC
    };

    struct RigidBody2D
    {
    public:
        // Adds force as impulse
        void AddImpulse(const glm::vec2 value)
        {
            body->ApplyLinearImpulse({value.x, value.y}, body->GetWorldCenter(), true);
        }

        // Sets instantaneous velocity 
        void SetVelocity(const glm::vec2 value)
        {
            body->SetLinearVelocity({value.x, value.y});
        }

    public:
        b2Body *body = nullptr;
        BodyType type = BodyType::DYNAMIC;
    };

    struct BoxCollider2D
    {
        float width = 1.0f;
        float height = 1.0f;
    };
};

#endif