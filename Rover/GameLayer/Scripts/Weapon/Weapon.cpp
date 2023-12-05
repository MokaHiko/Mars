#include "Weapon.h"
#include <Core/Time.h>

void Weapon::OnStart() 
{
}

void Weapon::OnUpdate(float dt) 
{
  _time_since_last_fire += mrs::Time::DeltaTime();
}

void Weapon::Equip(Ship* ship)
{
	_ship = ship;
	OnEquip();
}

void Weapon::Fire() 
{
	if(_time_since_last_fire >= _fire_rate)
	{
		_time_since_last_fire = 0;

		if(_hold_time >= _long_hold_threash_hold)
		{
			auto& projectile = CreateStrongProjectile(_hold_time);
		}
		else
		{
			auto& projectile = CreateWeakProjectile();
		}

		if(_ammo > 0)
		{
			_ammo--;
		}
	}
}

