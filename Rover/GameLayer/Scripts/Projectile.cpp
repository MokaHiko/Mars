#include "Projectile.h"
#include <Scripting/Process.h>

#include "Ship.h"
#include "Striker.h"

void Projectile::OnStart() 
{
	// Require components
	auto& rb = GetComponent<mrs::RigidBody2D>();
	auto& props = GetComponent<ProjectileProperties>();

	rb.SetVelocity(props.direction * props.speed);
}

void Projectile::OnUpdate(float dt) 
{
	auto& props = GetComponent<ProjectileProperties>();
	props.life_span -= dt;

	if(props.life_span <= 0)
	{
		QueueDestroy();
	}
}

void Projectile::OnCollisionEnter2D(mrs::Entity other) 
{
	// Do not collide with player ship
	if(other.GetScript<Ship>())
	{
		return;
	}

	// Check if enemy
	if(Striker* striker = other.GetScript<Striker>())
	{
		striker->TakeDamage(GetComponent<ProjectileProperties>().damage);
	}

	QueueDestroy();
}