#include "GravGun.h"
#include "Ship.h"

GravGun::GravGun()
{

}

GravGun::~GravGun()
{

}

void GravGun::FireStrongProjectile(float hold_time)
{
	auto& ship_transform = Owner()->GetComponent<mrs::Transform>();
	auto projectile = Instantiate("projectile!");
	// auto& transform = projectile.GetComponent<mrs::Transform>();

	// // TODO: Change to fire point
	// transform.position = ship_transform.position;
	// transform.rotation = ship_transform.rotation;
	// transform.position += ship_transform.up * 10.0f;
	// transform.scale *= 0.5f;

	// auto& props = projectile.AddComponent<ProjectileProperties>();
	// props.life_span = 5.0f;
	// props.speed = 30.0f;
	// props.damage = 50.0f;
	// props.owner = Owner()->_game_object;
	// props.direction = ship_transform.up;

	// projectile.AddComponent<mrs::MeshRenderer>(mrs::Mesh::Get("sphere"), mrs::Material::Get("default"));
	// auto& rb = projectile.AddComponent<mrs::RigidBody2D>();
	// rb.type = mrs::BodyType::KINEMATIC;
	// rb.use_gravity = false;
	// projectile.AddScript<Projectile>();
}
