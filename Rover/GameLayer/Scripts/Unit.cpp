#include "Unit.h"

void Unit::OnUpdate(float dt)
{
	if(!_to_destroy)
	{
		return;
	}

	_timer += dt;
	if(_timer > _death_timer)
	{
		QueueDestroy();
	}
}

void Unit::OnCollisionEnter2D(mrs::Entity other) 
{
	_to_destroy = true;
	_death_timer = static_cast<float>(rand() % 20);
}
