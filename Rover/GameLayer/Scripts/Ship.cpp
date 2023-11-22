#include "Ship.h"

#include <Core/Input.h>
#include <Physics/Physics.h>

#include "Projectile.h"
#include "ShipCombat.h"

#include "Scrap.h"

void Ship::OnCreate()
{
	// TODO: add required components

	// Initialize ship systems
	auto& combat = Instantiate("Combat");
	combat.AddComponent<mrs::Script>().Bind<ShipCombat>();

	auto& transform = GetComponent<mrs::Transform>();
	transform.AddChild(combat);
}

void Ship::OnUpdate(float dt)
{
}

void Ship::OnCollisionEnter2D(mrs::Entity other)
{
	// Check if resource
	if (other.HasComponent<Resource>())
	{
		const Resource& resource = other.GetComponent<Resource>();
		auto& ship_resources = GetComponent<ShipResources>();

		switch (resource.type)
		{
			case(ResourceType::CREDIT):
			{
				ship_resources.credit += resource.amount;
			}break;
			case(ResourceType::SCRAP_METAL):
			{
				ship_resources.scrap_metal += resource.amount;
			}break;
			default:
				break;
		}
	}
}

void Ship::Die()
{
	ShipResources& resources = GetComponent<ShipResources>();
	mrs::Vector3 position = GetComponent<mrs::Transform>().position;

	for (uint32_t i = 0; i < resources.scrap_metal; i++)
	{
		auto e = Instantiate("scrap_metal", position);

		auto& resource = e.AddComponent<Resource>();
		resource.type = ResourceType::SCRAP_METAL;
		resource.amount = 5.0f;

		e.GetComponent<mrs::Transform>().scale *= 0.25f;
		e.AddComponent<mrs::MeshRenderer>(mrs::Mesh::Get("cube"), mrs::Material::Get("default"));

		auto& rb = e.AddComponent<mrs::RigidBody2D>();
		rb.mass = 50.0f;
		rb.use_gravity = false;
		rb.type = mrs::BodyType::KINEMATIC;

		e.AddComponent<mrs::Script>().Bind<Scrap>();
	}

	QueueDestroy();
}

void Ship::TakeDamage(float damage)
{
	_health -= damage;

	if (_health <= 0)
	{
		Die();
	}
}