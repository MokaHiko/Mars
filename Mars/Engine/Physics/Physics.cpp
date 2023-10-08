#include "Physics.h"

#include "ECS/Entity.h"
#include "Core/Application.h"
#include "Physics2DLayer.h"

#include "Collider.h"

namespace mrs
{
	const Collision Physics::Raycast(Scene* scene, const Ray& ray, float range)
	{
		auto& view = scene->Registry()->view<Transform, MeshCollider>();
		glm::vec3 current_point = ray.origin;

		for(auto entity : view)
		{
			Entity e(entity, scene);
			auto& transform = e.GetComponent<Transform>();
			auto& col = e.GetComponent<MeshCollider>();

			Collision collision = {};
			collision.collision_points = col.collider->TestRayCollision(&transform, &ray);

			if(collision.collision_points.did_collide)
			{
				collision.entity = e;
				return collision;
			}
		}

		return Collision{};
	}
}
