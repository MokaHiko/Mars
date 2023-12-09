#include "Ship.h"

#include <Core/Input.h>
#include <Physics/Physics.h>

#include "Scrap.h"

#include "ShipCombat.h"
#include "ShipMovement.h"

#include "Effects/Effect.h"

#include "Weapon/ChargeCanon.h"

void Ship::OnStart() 
{
	auto e = Instantiate("ChargeCannon");
	auto& weapon = e.AddScript<ChargeCanon>();

	Combat().EquipWeapon(&weapon);
}

void Ship::OnCreate() {
  const auto &specs = GetComponent<ShipSpecs>();

  // Required components
  auto &rb = AddComponent<mrs::RigidBody2D>();
  rb.mass = 100;
  rb.use_gravity = false;
  AddComponent<ShipResources>();

  _ship_model = Instantiate("Ship Model");
  _ship_model.GetComponent<mrs::Transform>().rotation = {0, 0, 180};
  _ship_model.AddComponent<mrs::ModelRenderer>(specs.model);

  _ship_thrusters = Instantiate("thrusters_effect");
  _ship_thrusters.GetComponent<mrs::Transform>().position.x = 0.00f;
  _ship_thrusters.GetComponent<mrs::Transform>().position.y = 3.0f;
  auto &thruster_particles = _ship_thrusters.AddComponent<mrs::ParticleSystem>();
  thruster_particles.emission_rate = 32;
  thruster_particles.max_particles = 64;
  thruster_particles.particle_size = 1.25f;
  thruster_particles.spread_angle = 180.0f;
  thruster_particles.velocity = {7, 50};
  thruster_particles.color_1 = {0.093f, 0.747f, 0.265f, 1.000f};
  thruster_particles.color_2 = {0.231f, 0.000f, 0.391f, 0.000f};
  thruster_particles.life_time = 0.83f;
  thruster_particles.repeating = true;
  thruster_particles.world_space = true;
  thruster_particles.material = mrs::Material::Get("smoke");
  _ship_model.GetComponent<mrs::Transform>().AddChild((_ship_thrusters));

  // Initialize ship systems
  auto ship_movement = Instantiate("Movement");
  _ship_movement_script = &ship_movement.AddScript<ShipMovement>();
  _ship_movement_script->_ship = this;

  auto ship_combat = Instantiate("Combat");
  _ship_combat_script = &ship_combat.AddScript<ShipCombat>();
  ;
  _ship_combat_script->_ship = this;

  // Parent Systems
  auto &transform = GetComponent<mrs::Transform>();
  transform.AddChild(_ship_model);
  transform.AddChild(ship_movement);
  transform.AddChild(ship_combat);
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
	else if(other.HasComponent<ShipSpecs>())
	{
		auto e = Instantiate("Sparks", GetComponent<mrs::Transform>().position);

		auto& particles = e.AddComponent<mrs::ParticleSystem>();
		particles.repeating = false;

		particles.emission_shape = mrs::EmissionShape::Circle;
		particles.emission_rate = 32;
		particles.max_particles = 16;
		particles.velocity = mrs::Vector2{ 50.0f, 50.0f };
		particles.color_1 = mrs::Vector4(0.929, 0.075, 0.075, 1.0f);
		particles.color_2 = mrs::Vector4(0.749, 0.565, 0.043, 0.0f);
		particles.particle_size = 0.15f;
		particles.life_time = 3.0f;
		particles.duration = 5.0f;

		e.AddComponent<EffectProperties>().duration = particles.duration * 1.5f;
		e.AddScript<Effect>();

		TakeDamage(other.GetComponent<ShipSpecs>().mass / 4.0f);
	}
}

ShipCombat& Ship::Combat()
{
	return *_ship_combat_script;
}

ShipMovement& Ship::Movement()
{
	return *_ship_movement_script;
}

void Ship::Die()
{
	ShipResources& resources = GetComponent<ShipResources>();
	mrs::Vector3 position = GetComponent<mrs::Transform>().position;

	{
		auto e = Instantiate("Explosion", GetComponent<mrs::Transform>().position);

		auto& particles = e.AddComponent<mrs::ParticleSystem>();
		particles.material = mrs::Material::Get("smoke");

		particles.emission_shape = mrs::EmissionShape::Circle;
		particles.emission_rate = 64;
		particles.max_particles = 32;
		particles.velocity = mrs::Vector2{ 5.0f, 5.0f };
		particles.color_1 = mrs::Vector4(0.883f, 0.490f, 0.000f, 1.000f);
		particles.color_2 = mrs::Vector4(0.749, 0.565, 0.043, 0.25f);
		particles.particle_size = 3.0f;
		particles.life_time = 3.0f;
		particles.duration = 10.0f;
		particles.repeating = false;

		e.AddComponent<EffectProperties>().duration = particles.duration * 1.5f;
		e.AddScript<Effect>();
	}


	for (uint32_t i = 0; i < resources.scrap_metal / 2; i++)
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
		e.AddScript<Scrap>();
	}

	QueueDestroy();
}

void Ship::TakeDamage(float damage)
{
	ShipResources& resources = GetComponent<ShipResources>();
	resources.health -= damage;

	if (resources.health <= 0)
	{
		Die();
	}
}