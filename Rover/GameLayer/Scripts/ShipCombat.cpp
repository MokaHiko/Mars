#include "ShipCombat.h"
#include <Core/Input.h>

#include "Striker.h"

void ShipCombat::OnStart()
{
  _ship = GetComponent<mrs::Transform>().parent.GetScript<Ship>();

  for (auto e : FindEntitiesWithScript<Striker>())
  {
    if (auto enemy = e.GetComponent<mrs::Transform>().parent)
    {
      _targets.push_back(enemy);
    }
  }
}

void ShipCombat::OnUpdate(float dt) {
  if (mrs::Input::IsKeyDown(SDLK_SPACE))
  {
    for (auto e : _targets)
    {
      FireProjectile(e);
    }
  }
}

void ShipCombat::FireProjectile(mrs::Entity target)
{
  // Create projectile
  auto projectile = Instantiate("projectile!");
  auto& transform = projectile.GetComponent<mrs::Transform>();

  // TODO: Change to fire point
  transform.position = _ship->GetComponent<mrs::Transform>().position + mrs::Vector3(0, 5, 0);

  // TODO: Change to transform.up
  auto& props = projectile.AddComponent<ProjectileProperties>();
  props.direction = mrs::Vector2(0, 1);
  props.life_span = 5.0f;
  props.speed = 30.0f;
  props.damage = 100.0f;
  props.target = target;

  projectile.AddComponent<mrs::MeshRenderer>(mrs::Mesh::Get("sphere"), mrs::Material::Get("default"));
  auto& rb = projectile.AddComponent<mrs::RigidBody2D>();
  rb.type = mrs::BodyType::KINEMATIC;
  rb.use_gravity = false;

  projectile.AddComponent<mrs::Script>().Bind<Projectile>();

  auto particles_obj = Instantiate("particles!");
  auto& particles = particles_obj.AddComponent<mrs::ParticleSystem>();
  particles.emission_rate *= 15;
  particles.max_particles = 32;
  particles.velocity = mrs::Vector2{ 1, -50.0f };
  particles.color_1 = mrs::Vector4(0.91, 0.33, 0.1f, 1.0f);
  particles.color_2 = mrs::Vector4(0.91, 0.33, 0.1f, 1.0f) * 0.15f;
  particles.particle_size *= 5.0f;
  particles.life_time = 0.5f;

  transform.AddChild(particles_obj);
}

