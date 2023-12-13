#include "Physics.h"

#include "Core/Application.h"
#include "Physics2DLayer.h"

#include "Collider.h"

namespace mrs
{
	const Collision Physics::Raycast(Scene* scene, const Ray& ray, float range)
	{
		auto& view = scene->Registry()->view<Transform, MeshCollider>();
		glm::vec3 current_point = ray.origin;

		for (auto entity : view)
		{
			Entity e(entity, scene);
			auto& transform = e.GetComponent<Transform>();
			auto& col = e.GetComponent<MeshCollider>();

			Collision collision = {};
			collision.collision_points = col.collider->TestRayCollision(&transform, &ray);

			if (collision.collision_points.did_collide)
			{
				collision.entity = e;
				return collision;
			}
		}

		return Collision{};
	}

	void RigidBody2D::AddImpulse(const glm::vec2& value)
	{
		body->ApplyLinearImpulse({ value.x, value.y }, body->GetWorldCenter(), true);
	}

	Vector2 RigidBody2D::GetVelocity() const
	{
		if(!Initialized())
		{
			return start_velocity;
		}

		const auto& v = body->GetLinearVelocity();
		return { v.x, v.y };
	}

	float RigidBody2D::GetAngularVelocity() const
	{
		if(!Initialized())
		{
			return 0.0f;
		}

		return body->GetAngularVelocity();
	}

	void RigidBody2D::SetVelocity(const glm::vec2& value)
	{
		if(!Initialized())
		{
			start_velocity = value;
			return;
		}

		body->SetLinearVelocity({ value.x, value.y });
	}

	void RigidBody2D::SetType(BodyType t)
	{
		if(!Initialized())
		{
			type = t;
			return;
		}
		switch (type)
		{
		case BodyType::STATIC:
			body->SetType(b2BodyType::b2_staticBody);
			break;
		case BodyType::DYNAMIC:
			body->SetType(b2BodyType::b2_dynamicBody);
			break;
		case BodyType::KINEMATIC:
			body->SetType(b2BodyType::b2_kinematicBody);
			break;
		default:
			break;
		}
	}

	void RigidBody2D::SetTransform(const glm::vec2& position, const float angle)
	{
		body->SetTransform({ position.x, position.y }, angle);
	}

	void RigidBody2D::SetAngularVelocity(const float w)
	{
		if(!Initialized())
		{
			start_angular_velocity = w;
			return;
		}

		body->SetAngularVelocity(w);
	}

	void RigidBody2D::ApplyAngularImpulse(const float w)
	{
		body->ApplyAngularImpulse(w, true);
	}

	void RigidBody2D::SetFixedRotation(const bool flag)
	{
		body->SetFixedRotation(flag);
	};

	void Physics2D::Raycast(const Ray& ray, float range, std::function<void(Collision)> fn)
	{
		static Application& app = Application::Instance();
		static Physics2DLayer* physics = dynamic_cast<Physics2DLayer*>(app.FindLayer("Physics2DLayer"));
		Collision col;

		RayCastListener listener(app.GetScene(), fn);
		physics->Raycast(ray, range, listener);
	}
}
