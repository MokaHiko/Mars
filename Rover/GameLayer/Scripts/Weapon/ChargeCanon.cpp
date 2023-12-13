#include "ChargeCanon.h"
#include "Ship.h"

#include "Projectile/Bullet.h"
#include "Effects/Effect.h"
#include "Effects/TracerEffect.h"

#include <glm/gtx/quaternion.hpp>

tbx::PRNGenerator<float> ChargeCanon::random_gen(-1,1);

void ChargeCanon::OnStart()
{
  _fire_rate = 0.13f;
}

void ChargeCanon::FireWeakProjectile() {

  BulletMuzzleFlash();

  const auto& ship_transform = Owner()->GetComponent<mrs::Transform>();

  float recoil = 4.0f;

  mrs::Ray ray = {};
  ray.origin = ship_transform.position + (ship_transform.up * 10.0f); // Change to fire point

  glm::quat quat_rot = glm::quat(glm::vec3(0,0, glm::radians(recoil * random_gen.Next())));
  ray.direction = glm::rotate(quat_rot, ship_transform.up);

  float range = 500.0f;

  mrs::Vector3 target_pos = ray.origin + (ray.direction * range);
  mrs::Physics2D::Raycast(ray, range, [&](mrs::Collision& col)
    {
      if (Ship* ship = col.entity.GetScript<Ship>()) {
        target_pos = col.collision_points.a;

        auto weak_tracer_flash = Instantiate("Tracer");
        ship->TakeDamage(10.0f);

        auto e = Instantiate("hit_effect", col.collision_points.a);
        auto& transform = e.GetComponent<mrs::Transform>();
        transform.rotation.z += glm::degrees(glm::acos(glm::dot(transform.up, col.collision_points.normal)));

        auto& particles = e.AddComponent<mrs::ParticleSystem>();
        particles.duration = 5.0f;
        particles.repeating = false;

        particles.emission_shape = mrs::EmissionShape::Cone;
        particles.emission_rate = 64;
        particles.max_particles = 16;
        particles.spread_angle = 45;
        particles.velocity = mrs::Vector2{ 50.0f, 10.0f };
        particles.color_1 = mrs::Vector4(0.883, 0.490, 0.000, 1.000);
        particles.color_2 = mrs::Vector4(0.114, 0.054, 0.006, 0.000);
        particles.particle_size = 0.25f;
        particles.life_time = 0.85f;

        e.AddComponent<EffectProperties>().duration = particles.duration * 3.0f;
        e.AddScript<Effect>();
      }
    });

  // Tracers
  auto weak_tracer_flash = Instantiate("Tracer", ray.origin);
  auto& effect_props = weak_tracer_flash.AddComponent<EffectProperties>();
  effect_props.duration = 0.25f;
  effect_props.fixed_time = false;

  TracerEffect& tracer = weak_tracer_flash.AddScript<TracerEffect>();
  tracer.p1 = ray.origin;
  tracer.p2 = target_pos;
  tracer.range = range;
  tracer.target = Owner()->_game_object;
}

void ChargeCanon::FireStrongProjectile(float hold_time)
{
  auto& e = Instantiate("Bullet");
  e.AddScript<Bullet>()._owner = Owner()->_game_object;
}

void ChargeCanon::OnEquip()
{

}

void ChargeCanon::OnUnEquip()
{

}

mrs::Entity ChargeCanon::BulletMuzzleFlash()
{
  // TODO: Reset and Recycle 

  // Weapon muzzle Flash 
  auto& ship_model_transform = Owner()->_ship_model.GetComponent<mrs::Transform>();

  auto weak_muzzle_flash = Instantiate("Muzzle Flash");
  auto& weak_muzzle_flash_transform = weak_muzzle_flash.GetComponent<mrs::Transform>();
  weak_muzzle_flash_transform.rotation = ship_model_transform.rotation;
  weak_muzzle_flash_transform.position += ship_model_transform.up * 10.0f;
  weak_muzzle_flash_transform.position.z += 0.01f;

  auto& muzzle_particles = weak_muzzle_flash.AddComponent<mrs::ParticleSystem>();
  muzzle_particles.emission_rate = 128;
  muzzle_particles.max_particles = 16;
  muzzle_particles.particle_size = 3.2f;
  muzzle_particles.spread_angle = 45.0f;
  muzzle_particles.velocity = { 20,50 };
  muzzle_particles.color_1 = mrs::Vector4{ 0.883f, 0.490f, 0.000f, 1.000f };
  muzzle_particles.color_2 = mrs::Vector4{ 0.553f, 0.290f, 0.000f, 1.000f } *0.15f;
  muzzle_particles.life_time = 0.66f;
  muzzle_particles.repeating = false;
  muzzle_particles.duration = muzzle_particles.life_time * 5.0f;
  muzzle_particles.material = mrs::Material::Get("muzzle_02");

  weak_muzzle_flash.AddComponent<EffectProperties>().duration = muzzle_particles.duration;
  weak_muzzle_flash.AddScript<Effect>();
  ship_model_transform.AddChild(weak_muzzle_flash);

  return weak_muzzle_flash;
}
