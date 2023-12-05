#include "Projectile.h"
#include <Scripting/Process.h>

#include "Ship.h"
#include "Striker.h"
#include "Effects/Effect.h"

void Projectile::OnCreate()
{
}

void Projectile::OnStart() {
	Init();
	Launch();
}

void Projectile::OnUpdate(float dt)
{
	if (_time_alive >= _life_span)
	{
		Die();
	}

	_time_alive += dt;
}

void Projectile::OnCollisionEnter2D(mrs::Entity other)
{
	if (Ship* ship = other.GetScript<Ship>())
	{
		ship->TakeDamage(_damage);
	}

	Die();
}

void Projectile::Die()
{
	auto e = Instantiate("Explosion", GetComponent<mrs::Transform>().position);

	auto& particles = e.AddComponent<mrs::ParticleSystem>();
	particles.repeating = false;

	particles.emission_shape = mrs::EmissionShape::Circle;
	particles.emission_rate = 32;
	particles.max_particles = 32;
	particles.velocity = mrs::Vector2{ 5.0f, 5.0f };
	particles.color_1 = mrs::Vector4(0.91, 0.33, 0.1f, 1.0f);
	particles.color_2 = mrs::Vector4(0.91, 0.33, 0.1f, 1.0f) * 0.15f;
	particles.particle_size = 0.15f;
	particles.life_time = 6.0f;
	particles.duration = 3.0f;

	e.AddComponent<EffectProperties>().duration = particles.duration;
	e.AddScript<Effect>();

	QueueDestroy();
}