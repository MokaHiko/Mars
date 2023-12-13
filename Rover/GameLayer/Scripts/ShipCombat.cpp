#include "ShipCombat.h"
#include <Core/Input.h>

#include "Striker.h"
#include "Effects/Effect.h"

void ShipCombat::OnStart()
{
  _ship = GetComponent<mrs::Transform>().parent.GetScript<Ship>();
  ScanTargets();
}

void ShipCombat::OnUpdate(float dt)
{
}

void ShipCombat::SwitchCombatMode()
{
  if (_state == ShipCombatState::ManualCombat)
  {
    MRS_INFO("[ShipState]: AutoCombat");
    _state = ShipCombatState::AutoCombat;

    Ref<AutoCombatProcess> auto_combat = CreateRef<AutoCombatProcess>(*this);
    StartProcess(auto_combat);
  }
  else
  {
    MRS_INFO("[ShipState]: ManualCombat");
    _state = ShipCombatState::ManualCombat;
  }
}

void ShipCombat::EquipWeapon(Weapon* weapon)
{
  weapon->Equip(_ship);
  _current_weapon = weapon;
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

void ShipCombat::PrimaryFire() 
{
  _current_weapon->Fire();
}

void ShipCombat::SecondaryFire() 
{
  _current_weapon->Fire(1);
}

AutoCombatProcess::AutoCombatProcess(ShipCombat& combat)
  :_ship_combat(combat)
{
  _target_it = _ship_combat._targets.begin();
}

void AutoCombatProcess::OnUpdate(float dt)
{
  Succeed();
  // if (_target_it == _ship_combat._targets.end())
  // {
  //   Succeed();
  // }

  // _time_since_last_fire += dt;
  // if (_time_since_last_fire >= _ship_combat._fire_rate)
  // {
  //   auto e = *(_target_it);
  //   // Check if alive
  //   if (e && e.HasComponent<Target>())
  //   {
  //     _ship_combat.FireProjectile(e);
  //   }

  //   _time_since_last_fire = 0;
  //   _target_it++;
  // }
}


