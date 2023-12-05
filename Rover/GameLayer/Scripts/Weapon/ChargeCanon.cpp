#include "ChargeCanon.h"
#include "Ship.h"

#include "Projectile/Bullet.h"
#include "Effects/Effect.h"

mrs::Entity ChargeCanon::CreateWeakProjectile()
{
	// TODO: Recycle 
	// Weapon muzzle Flash 
	auto& ship_model_transform = Owner()->_ship_model.GetComponent<mrs::Transform>();

	auto weak_muzzle_flash = Instantiate("Muzzle Flash");
	auto& weak_muzzle_flash_transform = weak_muzzle_flash.GetComponent<mrs::Transform>();
	weak_muzzle_flash_transform.rotation = ship_model_transform.rotation;

	//weak_muzzle_flash_transform.position = ship_model_transform.position;
	weak_muzzle_flash_transform.position += ship_model_transform.up * 10.0f;

	auto& muzzle_particles = weak_muzzle_flash.AddComponent<mrs::ParticleSystem>();
	muzzle_particles.emission_rate = 128;
	muzzle_particles.max_particles = 32;
	muzzle_particles.particle_size = 1.25f;
	muzzle_particles.spread_angle = 45.0f;
	muzzle_particles.velocity = { 15,50 };
	muzzle_particles.color_1 = mrs::Vector4{ 1.0, 1.006, 1, 1.0f };
	muzzle_particles.color_2 = mrs::Vector4{ 1.0, 1.006, 1, 1.0f } * 0.10f;
	muzzle_particles.life_time = 1.333f;
	muzzle_particles.repeating = false;
	//muzzle_particles.emission_shape = mrs::EmissionShape::Circle;
	muzzle_particles.duration = muzzle_particles.life_time * 5.0f;
	muzzle_particles.material = mrs::Material::Get("smoke");

	weak_muzzle_flash.AddComponent<EffectProperties>().duration = muzzle_particles.duration;
	weak_muzzle_flash.AddScript<Effect>();
	ship_model_transform.AddChild(weak_muzzle_flash);

	auto& e = Instantiate("Bullet");
	e.AddScript<Bullet>()._owner = Owner()->_game_object;

	return e;
}

mrs::Entity ChargeCanon::CreateStrongProjectile(float hold_time)
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

	// auto rocket_thrusters = Instantiate("thrusters_effect");
	// rocket_thrusters.GetComponent<mrs::Transform>().position.y = -3.0f;
	// auto &thruster_particles = rocket_thrusters.AddComponent<mrs::ParticleSystem>();
	// thruster_particles.emission_rate = 16;
	// thruster_particles.max_particles = 48;
	// thruster_particles.particle_size = 2.0f;
	// thruster_particles.spread_angle = 20.0f;
	// thruster_particles.velocity = mrs::Vector2{-props.speed, -props.speed} * 0.75f;
	// thruster_particles.color_1= mrs::Vector4{1.0f};
	// thruster_particles.color_2 = mrs::Vector4{1.0f} *= 0.15f;
	// thruster_particles.life_time = 1.66f;
	// thruster_particles.repeating = true;
	// thruster_particles.material = mrs::Material::Get("smoke");
	// transform.AddChild(rocket_thrusters);
	return projectile;
}

void ChargeCanon::OnEquip()
{

}

void ChargeCanon::OnUnEquip()
{

}
