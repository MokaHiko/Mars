#include "Planet.h"

void Planet::OnStart() 
{

}

void Planet::OnUpdate(float dt) 
{
	const auto& props = GetComponent<PlanetProperties>();
	GetComponent<mrs::Transform>().rotation += props.axis_of_rotation * props.rotation_rate * dt;
}
