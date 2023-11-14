#include "Ship.h"

#include <Core/Input.h>
#include <Physics/Physics.h>

#include "Projectile.h"

void Ship::OnCreate() {}

void Ship::OnStart() 
{

}


void Ship::OnUpdate(float dt)
{
	// TODO: Move to Ship Controller
	mrs::Transform& transform = GetComponent<mrs::Transform>();
	mrs::RigidBody2D& rb = GetComponent<mrs::RigidBody2D>();

	static float ms = 50.0f;
	static float total_time = 0;
	static float rotation_duration = 1.0f;
	static float target_rotation = 45.0f;

	glm::vec2 velocity = {0, 0};

	if (mrs::Input::IsKeyDown(SDLK_SPACE))
	{
		FireProjectile();
	}

	else if (mrs::Input::IsKeyPressed(SDLK_q))
	{
	}

	if (mrs::Input::IsKeyPressed(SDLK_w))
	{
		velocity += glm::vec2{0, ms};
	}
	else if (mrs::Input::IsKeyPressed(SDLK_s))
	{
		velocity += glm::vec2{0, -ms};
	}

	if (mrs::Input::IsKeyPressed(SDLK_d))
	{
		velocity += glm::vec2{ms, 0};

		total_time += dt;
		transform.rotation.y = Lerp(transform.rotation.y, target_rotation, total_time / rotation_duration);
		if(target_rotation - transform.rotation.y < 0.01f)
		{
			total_time = 0;
		}
	}
	else if (mrs::Input::IsKeyPressed(SDLK_a))
	{
		velocity += glm::vec2{-ms, 0};

		total_time += dt;
		transform.rotation.y = Lerp(transform.rotation.y, -target_rotation, total_time / rotation_duration);

		if(-target_rotation - transform.rotation.y < 0.01f)
		{
			total_time = 0;
		}
	}

	rb.SetVelocity(velocity);
}

void Ship::OnCollisionEnter2D(mrs::Entity other) {}

void Ship::FireProjectile() 
{
	// Create projectile
	auto projectile = Instantiate("projectile!");
	auto& transform = projectile.GetComponent<mrs::Transform>();

	// TODO: Change to fire point
	transform.position = GetComponent<mrs::Transform>().position + mrs::Vector3(0, 5, 0);

	// TODO: Change to transform.up
	auto& props = projectile.AddComponent<ProjectileProperties>();
	props.direction = mrs::Vector2(0,1);
	props.life_span = 5.0f;
	props.speed = 100.0f;
	props.damage = 100.0f;

	projectile.AddComponent<mrs::MeshRenderer>(mrs::Mesh::Get("sphere"), mrs::Material::Get("default"));
	auto& rb = projectile.AddComponent<mrs::RigidBody2D>();
	rb.use_gravity = false;

	projectile.AddComponent<mrs::Script>().Bind<Projectile>();
}
