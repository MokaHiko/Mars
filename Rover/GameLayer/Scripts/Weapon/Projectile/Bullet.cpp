#include "Bullet.h"

void Bullet::Init() 
{
	auto& ship_transform = _owner.GetComponent<mrs::Transform>();
	auto& transform = GetComponent<mrs::Transform>();
	transform.scale *= 0.5f;

	// Projectile Properties
	_life_span = 2.0f;
	_speed = 100.0f;
	_damage = 10.0f;
	_direction = ship_transform.up;

	// TODO: Change to fire point
	transform.position = ship_transform.position;
	transform.rotation = ship_transform.rotation;
	transform.position += ship_transform.up * 10.0f;

	AddComponent<mrs::MeshRenderer>(mrs::Mesh::Get("cube"), mrs::Material::Get("default"));
	auto& rb = AddComponent<mrs::RigidBody2D>();
	rb.type = mrs::BodyType::KINEMATIC;
	rb.use_gravity = false;
}

void Bullet::Launch() 
{
	auto& rb = GetComponent<mrs::RigidBody2D>();
	mrs::Vector2 velocity = _owner.GetComponent<mrs::RigidBody2D>().GetVelocity() + (_direction * _speed); 
	rb.SetVelocity(velocity);
}
