#include "Striker.h"

void Striker::TakeDamage(float damage) 
{
	_health -= damage;
	if(_health <= 0)
	{
		QueueDestroy();
	}
}
