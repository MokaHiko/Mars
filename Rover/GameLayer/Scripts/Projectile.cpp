#include "Projectile.h"
#include <Scripting/Process.h>

#include "Ship.h"
#include "Striker.h"
#include "Effect.h"

void Projectile::OnStart()
{
	// Require components
	auto& rb = GetComponent<mrs::RigidBody2D>();
	auto& props = GetComponent<ProjectileProperties>();

	_start_pos = mrs::Vector2(GetComponent<mrs::Transform>().position);

	// For non tracking projectiles
	//_target_pos = _start_pos + (props.direction * 10.0f);
}

void Projectile::OnUpdate(float dt)
{
	auto& props = GetComponent<ProjectileProperties>();
	props.life_span -= dt;

	if (props.life_span <= 0)
	{
		Die();
	}

	// Check and clear target if not alive or not target
	if (!props.target || !props.target.HasComponent<Target>())
	{
		props.target = {};

		const mrs::Transform& transform = GetComponent<mrs::Transform>();
		GetComponent<mrs::RigidBody2D>().SetVelocity(transform.up * props.speed);
		return;
	}

	float launch_animation_time = 0.33f; 
	if(_time_alive <= launch_animation_time)
	{
		mrs::Transform& transform = GetComponent<mrs::Transform>();
		mrs::Vector2 pos = mrs::Vector2(transform.position);

		mrs::Vector2 target_pos = _start_pos + (mrs::Vector2(transform.up) * 5.0f);
		mrs::Vector2 next_pos = mrs::Slerp(_start_pos, target_pos, _time_alive / launch_animation_time, props.side > 0);

		GetComponent<mrs::RigidBody2D>().SetTransform(next_pos, 0);
	}
	else 
	{
		mrs::Transform& transform = GetComponent<mrs::Transform>();
		mrs::Vector2 pos = mrs::Vector2(transform.position);

		mrs::Vector2 target_pos = mrs::Vector2(props.target.GetComponent<mrs::Transform>().position);
		mrs::Vector2 dir = glm::normalize(target_pos - pos);

		GetComponent<mrs::RigidBody2D>().SetVelocity(dir * props.speed);
	}


	_time_alive += dt;
}

void Projectile::OnCollisionEnter2D(mrs::Entity other)
{
	if (Ship* ship = other.GetScript<Ship>())
	{
		ship->TakeDamage(GetComponent<ProjectileProperties>().damage);
	}

	Die();
}

void Projectile::Die()
{
	auto e = Instantiate("Explosion", GetComponent<mrs::Transform>().position);

	auto& particles = e.AddComponent<mrs::ParticleSystem>();
	particles.emission_shape = mrs::EmissionShape::Circle;
	particles.emission_rate = 32;
	particles.max_particles = 64;
	particles.velocity = mrs::Vector2{ 100.0f, 100.0f };
	particles.color_1 = mrs::Vector4(0.91, 0.33, 0.1f, 1.0f);
	particles.color_2 = mrs::Vector4(0.91, 0.33, 0.1f, 1.0f) * 0.15f;
	particles.particle_size = 0.75f;
	particles.life_time = 0.85f;
	particles.repeating = false;

	e.AddComponent<EffectProperties>().duration = particles.life_time * 1.5f;
	e.AddComponent<mrs::Script>().Bind<Effect>();

	QueueDestroy();
}
