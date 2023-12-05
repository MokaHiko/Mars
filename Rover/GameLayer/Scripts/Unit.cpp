#include "Unit.h"

#include <ECS/Components/Components.h>
#include <Core/Time.h>
#include <Toolbox/RandomToolBox.h>

#include <glm/gtx/compatibility.hpp>

void Unit::OnCreate() {}

void Unit::OnStart() 
{

}

void Unit::OnUpdate(float dt) 
{
	_duration -= dt;
	if(_duration <= 0)
	{
		QueueDestroy();
	}
}

void Unit::OnCollisionEnter2D(mrs::Entity other) 
{

}


