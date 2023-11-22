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

	auto& pos = GetComponent<mrs::Transform>().position;
	_start_pos = mrs::Vector2(pos.x, pos.y);

	// For non tracking projectiles
	//_target_pos = _start_pos + (props.direction * 10.0f);
}

void Projectile::OnUpdate(float dt) 
{
	auto& props = GetComponent<ProjectileProperties>();
	props.life_span -= dt;

	if(props.life_span <= 0)
	{
		Die();
	}

	if(_time_alive >= _duration)
	{
		// Check andd clear target if not alive
		if(!props.target.IsAlive())
		{
			props.target = {};
			return;
		}

		mrs::Transform& transform = GetComponent<mrs::Transform>();
		mrs::Vector2 pos = mrs::Vector2(transform.position.x, transform.position.y);

		mrs::Vector3 target_pos = props.target.GetComponent<mrs::Transform>().position;
		mrs::Vector2 dir = glm::normalize(mrs::Vector2{target_pos.x, target_pos.y} - pos);

		// TODO: Check if projectile tick time is reached to change directions
		GetComponent<mrs::RigidBody2D>().SetVelocity(dir * props.speed);
	}
	else
	{
		// TODO: Change to launch animation coroutine
		mrs::Transform& transform = GetComponent<mrs::Transform>();
		mrs::Vector2 pos = mrs::Vector2(transform.position.x, transform.position.y);

		// Change to fixed launch point
		mrs::Vector2 target_pos = pos + mrs::Vector2(0, 10);
		mrs::Vector2 next_pos = mrs::Slerp(_start_pos, target_pos, _time_alive / _duration, props.side);

		mrs::Vector2 dir = next_pos - pos;
		GetComponent<mrs::RigidBody2D>().SetVelocity(dir);
	}

	_time_alive += dt;
}

void Projectile::OnCollisionEnter2D(mrs::Entity other) 
{
	if(Ship* ship = other.GetScript<Ship>())
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
	particles.emission_rate *= 15;
	particles.max_particles = 32;
	particles.velocity = mrs::Vector2{1, -50.0f};
	particles.color_1 = mrs::Vector4(0.91, 0.33, 0.1f, 1.0f);
	particles.color_2 = mrs::Vector4(0.91, 0.33, 0.1f, 1.0f) * 0.15f;
	particles.particle_size *= 5.0f;
	particles.life_time = 0.5f;
	particles.repeating = false;

	e.AddComponent<EffectProperties>().duration = 1.0f;
	e.AddComponent<mrs::Script>().Bind<Effect>();

	QueueDestroy();
}
