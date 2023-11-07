#include "Planet.h"

Planet::Planet()
{

}

Planet::~Planet()
{

}

void Planet::OnStart() 
{

}

void Planet::OnUpdate(float dt) 
{
	float rate = 45;
	GetComponent<mrs::Transform>().rotation.z += rate * dt;
}
