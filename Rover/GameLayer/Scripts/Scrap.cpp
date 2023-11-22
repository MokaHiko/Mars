#include "Scrap.h"

#include <Scripting/Process.h>
#include "Ship.h"

tbx::PRNGenerator<float> Scrap::_random = {0, 2.0f * 3.14159265f};

void Scrap::OnStart() 
{
	float angle = _random.Next();
	float x = mrs::Cos(angle);
	float y = mrs::Sin(angle);

	auto& rb = GetComponent<mrs::RigidBody2D>();
	rb.SetVelocity(mrs::Vector2(x,y) * _explosion_magnitude);
	rb.SetAngularVelocity(sin(_random.Next()) * _explosion_magnitude);

	Ref<mrs::DelayProcess> delay_destroy = CreateRef<mrs::DelayProcess>(_life_span, [&](){
		Fade();
	});

	StartProcess(delay_destroy);
}

void Scrap::OnCollisionEnter2D(mrs::Entity other) 
{
	if(other.HasComponent<ShipResources>())
	{
		Fade();
	}
}

void Scrap::Fade() 
{ 
	QueueDestroy(); 
}
