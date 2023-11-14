#include "Moon.h"

void Moon::OnStart() 
{
}

void Moon::OnUpdate(float dt) 
{
	mrs::Vector2 next = {};
	auto& props = GetComponent<MoonProperties>();
	auto& planet_transform = props._planet.GetComponent<mrs::Transform>();
	const auto& planet_position = planet_transform.position;

	theta += dt;
	next.x = planet_position.x + (props.a * mrs::Cos(theta));
	next.y = planet_position.y + (props.b * mrs::Sin(theta));

	auto& transform = GetComponent<mrs::Transform>();
	transform.position.x = next.x;
	transform.position.y = next.y;
}
