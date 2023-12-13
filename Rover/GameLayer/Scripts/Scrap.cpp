#include "Scrap.h"

#include <Scripting/Process.h>
#include "Ship.h"

tbx::PRNGenerator<float> Scrap::_random_angle = {0, 2.0f * 3.14159265f};
tbx::PRNGenerator<float> Scrap::_random = {0, 1};

void Scrap::OnStart() 
{
	float angle = _random_angle.Next();
	float x = mrs::Cos(angle);
	float y = mrs::Sin(angle);

	float variance = 0.25f;
	float r = _explosion_magnitude * (1 + (variance * _random.Next()));

	auto& rb = GetComponent<mrs::RigidBody2D>();
	rb.SetVelocity(mrs::Vector2(x,y) * (r/_explosion_magnitude));
	rb.SetAngularVelocity(sin(_random_angle.Next()) * _explosion_magnitude);

	Ref<mrs::DelayProcess> delay_destroy = CreateRef<mrs::DelayProcess>(_life_span, [&](){
		Fade();
	});

	StartProcess(delay_destroy);
}

void Scrap::OnCollisionEnter2D(mrs::Collision& col) 
{
	if(col.entity.HasComponent<ShipResources>())
	{
		Fade();
	}
}

void Scrap::Fade() 
{ 
	QueueDestroy(); 
}
