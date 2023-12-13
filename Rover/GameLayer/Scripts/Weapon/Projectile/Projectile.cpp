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

void Projectile::OnCollisionEnter2D(mrs::Collision& col)
{
	if (Ship* ship = col.entity.GetScript<Ship>())
	{
		ship->TakeDamage(_damage);
	}

	auto e = Instantiate("Projectile Explosion", GetComponent<mrs::Transform>().position);
	auto& particles = e.AddComponent<mrs::ParticleSystem>();
	particles.duration = 6.0f;
	particles.repeating = false;

	particles.emission_shape = mrs::EmissionShape::Circle;
	particles.emission_rate = 32;
	particles.max_particles = 16;
	particles.velocity = mrs::Vector2{ 10.0f, 10.0f };
    particles.color_1 = mrs::Vector4(0.883, 0.490, 0.000, 1.000);
    particles.color_2 = mrs::Vector4(0.114, 0.054, 0.006, 0.000);
	particles.particle_size = 0.25f;
	particles.life_time = 3.0f;
	particles.duration = 5.0f;

	e.AddComponent<EffectProperties>().duration = particles.duration * 3.0f;
	e.AddScript<Effect>();

	Die();
}

void Projectile::Die()
{
	QueueDestroy();
}