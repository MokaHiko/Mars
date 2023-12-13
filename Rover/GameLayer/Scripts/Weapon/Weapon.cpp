#include "Weapon.h"
#include <Core/Time.h>

void Weapon::OnUpdate(float dt) 
{
   _time_since_last_primary_fire += mrs::Time::DeltaTime();
   _time_since_last_secondary_fire += mrs::Time::DeltaTime();
}

void Weapon::Equip(Ship* ship)
{
	_ship = ship;
	OnEquip();
}

void Weapon::Fire(int alternate_fire) 
{
	if(!alternate_fire)
	{
		if(_time_since_last_primary_fire >= _primary_fire_rate)
		{
			_time_since_last_primary_fire = 0;

			FireWeakProjectile();
			if(_secondary_ammo > 0)
			{
				_secondary_ammo--;
			}
		}
	}
	else
	{
		if(_time_since_last_secondary_fire >= _secondary_fire_rate)
		{
			_time_since_last_secondary_fire = 0;

			FireStrongProjectile(100.0f);
			if(_secondary_ammo > 0)
			{
				_secondary_ammo--;
			}
		}
	}
}

