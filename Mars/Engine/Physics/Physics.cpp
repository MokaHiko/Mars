#include "Physics.h"

#include "ECS/Entity.h"
#include "Core/Application.h"
#include "Physics2DLayer.h"

namespace mrs
{
    template <>
    RigidBody2D& Entity::AddComponent<RigidBody2D>()
    {
        if (HasComponent<RigidBody2D>())
        {
            return GetComponent<RigidBody2D>();
        }

        auto& rb =_scene->_registry.emplace<RigidBody2D>(this->_id);

        auto& app = Application::GetInstance();

        // Real time addition if layer is active
        Physics2DLayer* physics_layer = static_cast<Physics2DLayer*>(app.FindLayer("Physics2DLayer"));
        if(physics_layer->IsEnabled())
        {
            physics_layer->AddBody(*this);
        }

        return rb;
    }
}
