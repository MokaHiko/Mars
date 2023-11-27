#include "ShipCombat.h"
#include <Core/Input.h>

#include "Striker.h"

void ShipCombat::OnStart()
{
  _ship = GetComponent<mrs::Transform>().parent.GetScript<Ship>();
  ScanTargets();
}

void ShipCombat::OnUpdate(float dt)
{
  ProcessInput();
}

void ShipCombat::ProcessInput()
{
  // Automated combat while toggling space
  // TODO: add indicator that automatic combat is on
  if (mrs::Input::IsKeyDown(SDLK_SPACE))
  {
    if (_state == ShipCombatState::Idle)
    {
      MRS_INFO("[ShipState]: AutoCombat");
      _state = ShipCombatState::AutoCombat;

      Ref<AutoCombatProcess> auto_combat = CreateRef<AutoCombatProcess>(*this);
      StartProcess(auto_combat);
    }
    else
    {
      MRS_INFO("[ShipState]: Idle");
      _state = ShipCombatState::Idle;
    }
  }
}

void ShipCombat::FireProjectile(mrs::Entity target)
{
  // Create projectile
  auto projectile = Instantiate("projectile!");
  auto& transform = projectile.GetComponent<mrs::Transform>();

  // TODO: Change to fire point
  transform.position = _ship->GetComponent<mrs::Transform>().position;
  transform.position += _ship->GetComponent<mrs::Transform>().up * 5.0f;

  // TODO: Define in weapon
  auto& props = projectile.AddComponent<ProjectileProperties>();
  props.direction = mrs::Vector2(0, 1);
  props.life_span = 5.0f;
  props.speed = 30.0f;
  props.damage = 20.0f;

  static int s = 1;
  props.side = s;
  MRS_INFO("%s", props.side > 0 ? "LEFT" : "RIGHT");
  s *= -1;

  props.target = target;

  projectile.AddComponent<mrs::MeshRenderer>(mrs::Mesh::Get("sphere"), mrs::Material::Get("default"));
  auto& rb = projectile.AddComponent<mrs::RigidBody2D>();
  rb.type = mrs::BodyType::KINEMATIC;
  rb.use_gravity = false;

  projectile.AddComponent<mrs::Script>().Bind<Projectile>();

  auto particles_obj = Instantiate("particles!", {0, -3, 0});
  auto& particles = particles_obj.AddComponent<mrs::ParticleSystem>();
  particles.emission_rate = 16;
  particles.max_particles = 64;
  particles.particle_size = 0.75f;
  particles.spread_angle = 25.0f;
  particles.velocity = { -50.0f, -25.0f };
  particles.color_1 = { 0.883, 0.490, 0.000, 1.000 };
  particles.color_2 = { 0.114, 0.054, 0.006, 0.000 };
  particles.life_time = 0.88f;
  particles.repeating = true;

  transform.AddChild(particles_obj);
}

void ShipCombat::ScanTargets()
{
  _targets.clear();
  for (auto e : FindEntitiesWithScript<Striker>())
  {
    if (auto enemy = e.GetComponent<mrs::Transform>().parent)
    {
      // Mark target for tracking
      enemy.AddComponent<Target>();
      _targets.push_back(enemy);
    }
  }

  // TODO: Add scanning vfx
}

AutoCombatProcess::AutoCombatProcess(ShipCombat& combat)
  :_ship_combat(combat)
{
  _target_it = _ship_combat._targets.begin();
}

void AutoCombatProcess::OnUpdate(float dt)
{
  if (_target_it == _ship_combat._targets.end())
  {
    Succeed();
  }

  _time_since_last_fire += dt;
  if (_time_since_last_fire >= _ship_combat._fire_rate)
  {
    auto e = *(_target_it);
    // Check if alive
    if (e && e.HasComponent<Target>())
    {
      _ship_combat.FireProjectile(e);
    }

    _time_since_last_fire = 0;
    _target_it++;
  }
}


