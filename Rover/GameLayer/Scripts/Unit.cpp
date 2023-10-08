#include "Unit.h"

#include <ECS/Components/Components.h>
#include <Core/Time.h>
#include <Toolbox/RandomToolBox.h>

#include <glm/gtx/compatibility.hpp>

void Unit::OnCreate() {}

void Unit::OnStart() 
{
	static tbx::PRNGenerator<float> random(-50, 50);
	_target_position = glm::vec2(random.Next(), random.Next());
}

void Unit::OnUpdate(float dt) 
{
	MoveTowards(_target_position);
}

void Unit::OnCollisionEnter2D(mrs::Entity other) 
{
	if(_health <= 0.0f)
	{
		return;
	}

	_health -= 50.0f;
	if(_health <= 0.0f)
	{
		//Die();
	}
}

void Unit::MoveTo(const glm::vec2& position)
{
	_target_position = position;
}

void Unit::MoveTowards(const glm::vec2& position)
{
	// TODO: use to a process based system
	auto& transform = GetComponent<mrs::Transform>();
	auto& rb = GetComponent<mrs::RigidBody2D>();

	glm::vec2 diff = position - glm::vec2(transform.position.x, transform.position.y);
	const float magnitude = glm::length(diff);

	if(magnitude < 0.5f)
	{
		rb.SetVelocity({0, 0});
		rb.SetAngularVelocity(0);
		return;
	}

	glm::vec2 velocity = glm::normalize(diff) * _movement_speed;
	rb.SetVelocity(velocity);
}

void Unit::Die() 
{
	QueueDestroy();
}
