#include "Bullet.h"

void Bullet::Init()
{
	auto& ship_transform = _owner.GetComponent<mrs::Transform>();
	auto& transform = GetComponent<mrs::Transform>();
	transform.scale *= 0.5f;

	// Projectile Properties
	_life_span = 2.0f;
	_speed = 100.0f;
	_damage = 25.0f;
	_direction = ship_transform.up;

	// TODO: Change to fire point
	transform.position = ship_transform.position;
	transform.position += ship_transform.up * 10.0f;

	AddComponent<mrs::MeshRenderer>(mrs::Mesh::Get("cube"), mrs::Material::Get("default"));
	auto& rb = AddComponent<mrs::RigidBody2D>();
	rb.type = mrs::BodyType::KINEMATIC;
	rb.use_gravity = false;
	rb.mass = 0.15f;

	auto& trail = AddComponent<mrs::ParticleSystem>();
	trail.emission_rate = 32;
	trail.max_particles = 64;
	trail.particle_size = 0.15f;
	trail.spread_angle = 35.0f;
	trail.velocity = { 10, 50 };
	trail.color_1 = { 1.0f, 1.0f, 1.0f, 1.000f };
	trail.color_2 = { 0.0f, 0.0f, 0.0f, 0.25f};
	trail.life_time = 0.55f;
	trail.repeating = true;
	trail.world_space = true;
}

void Bullet::Launch()
{
	auto& rb = GetComponent<mrs::RigidBody2D>();
	auto& owner_rb = _owner.GetComponent<mrs::RigidBody2D>();
	mrs::Vector2 velocity = owner_rb.GetVelocity() + (_direction * _speed);

	rb.SetVelocity(velocity);
}
