#include "Algorithm.h"
#include "ECS/Components/Components.h"
#include <glm/gtc/matrix_transform.hpp>

mrs::CollisionPoints mrs::algo::FindRaySphereCollision(const Ray* ray, const SphereCollider* col_b, const Transform* transform_b)
{
	float radius = col_b->radius;
	glm::vec3 v = ray->origin - transform_b->position;
	float a = glm::dot(ray->direction, ray->direction);
	float b = 2 * glm::dot(ray->direction, v);
	float c = glm::dot(v, v) - (radius * radius);

	float desc = (b * b) - (4 * a * c);
	if (desc > 0)
	{
		float desc_root = sqrt(desc);
		float t1 = (-b + desc_root) / (2 * a);
		float t2 = (-b - desc_root) / (2 * a);
		glm::vec3 inter_a = ray->origin + t1 * ray->direction;
		glm::vec3 inter_b = ray->origin + t2 * ray->direction;

		CollisionPoints col_points = {};
		glm::vec3 diff = transform_b->position - inter_a;

		col_points.a = inter_a;
		col_points.b = inter_b;
		col_points.normal = glm::normalize(diff);
		col_points.depth = glm::length(diff);
		col_points.did_collide = true;

		return col_points;
	}
	else
	{
		return CollisionPoints();
	}
}

mrs::CollisionPoints mrs::algo::FindRayPlaneCollision(const Ray* ray, const PlaneCollider* col_b, const Transform* transform_b)
{
	glm::vec3 rotation = transform_b->rotation;
	glm::mat4 rot_matrix(1.0f);
	rot_matrix = glm::rotate(rot_matrix, glm::radians(rotation.x), {1,0,0});

	float sigma = glm::length(transform_b->position);
	glm::vec4 normal_h = rot_matrix * glm::vec4(col_b->normal.x, col_b->normal.y, col_b->normal.z , 1.0f);
	glm::vec3 normal = glm::vec3(normal_h.x, normal_h.y, normal_h.z);

	float t = (-(sigma + glm::dot(normal, ray->origin))) / glm::dot(ray->direction, normal);
	if(t > 0)
	{
		glm::vec3 col_point = ray->origin + (t * ray->direction);
		glm::vec3 diff = (col_point - transform_b->position);

		// Planes are uniformly scaled
		if(glm::length(diff) <= transform_b->scale.x / 2.0f)
		{
			CollisionPoints col = {};
			col.did_collide = true;
			col.a = col_point;
			col.depth = t;
			
			return col;
		}
	}
	
	return CollisionPoints();
}
