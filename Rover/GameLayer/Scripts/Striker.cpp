#include "Striker.h"
#include "Core/Time.h"

#include "ShipCombat.h"
#include "ShipMovement.h"

#include <Toolbox/RandomToolBox.h>

tbx::PRNGenerator<float> prn_gen(0.0f, 1.0f);

void Striker::OnStart() 
{
	_ship = GetComponent<mrs::Transform>().parent.GetScript<Ship>();

	// Initialize States
	_idle_state = CreateRef<StrikerIdleState>(this);
	_patrol_state = CreateRef<StrikerPatrolState>(this);

	// Default State
	SetState(_patrol_state);
}

void Striker::OnCreate() 
{
}

void Striker::OnUpdate(float dt) 
{
	if(_since_last_think >= _tick_rate)
	{
		Think();
	}
	_since_last_think += dt;

	if(!_state)
	{
		Think();
		return;
	}

	_state->Update();
}

void Striker::Think() 
{
	// Params to determine State
}

void Striker::SetState(Ref<AIState> new_state) 
{
	if(!new_state)
	{
		MRS_INFO("Invalid State!");
	}

	// Call last state on end
	if (_state)
	{
		_state->OnEnd();
	}

	_state = new_state;
	_state->OnBegin();
}

StrikerIdleState::StrikerIdleState(Striker* striker)
	:StrikerState(striker)
{
}

void StrikerIdleState::OnBegin() 
{
}

void StrikerIdleState::OnEnd() 
{
}

void StrikerIdleState::Update() 
{
}

StrikerPatrolState::StrikerPatrolState(Striker* striker)
	:StrikerState(striker) {}

void StrikerPatrolState::OnBegin() 
{
	const auto& ship_transform = _ship->GetComponent<mrs::Transform>();
	const mrs::Vector2 _start_pos = mrs::Vector2(ship_transform.position);

	for(int i = 0; i < 2; i++)
	{
		float r = (_patrol_range * prn_gen.Next());
		float angle = glm::radians(360.0f) * prn_gen.Next();

		float x = r * glm::cos(angle);  
		float y = r * glm::sin(angle);  

		_patrol_points.push_back(_start_pos + mrs::Vector2(x, y));
	}

	for(const auto& p : _patrol_points)
	{
		// auto p1 = _striker->Instantiate("Target");
		// p1.GetComponent<mrs::Transform>().position = mrs::Vector3(p.x, p.y, ship_transform.position.z);
		// p1.GetComponent<mrs::Transform>().scale *= 2.0f;
		// p1.AddComponent<mrs::MeshRenderer>();
	}

	_current_patrol_point = 0;
}

void StrikerPatrolState::OnEnd() {}

void StrikerPatrolState::Update() 
{
	_time_elapsed += mrs::Time::DeltaTime();

	mrs::Vector2 ship_pos = _ship->GetComponent<mrs::Transform>().position;
	mrs::Vector2 diff = _patrol_points[_current_patrol_point] - mrs::Vector2(ship_pos);

	float diff_mag = glm::length(diff);
	if(diff_mag <= 2.5f)
	{
		_current_patrol_point = (_current_patrol_point + 1) % static_cast<int>(_patrol_points.size());
		return;
	}

	mrs::Vector2 dir = glm::normalize(diff);
	float dot = glm::dot(dir, mrs::Vector2(_ship->GetComponent<mrs::Transform>().up));
	float cross = glm::length(glm::cross(mrs::Vector3(dir, 0), _ship->GetComponent<mrs::Transform>().up));

	if(dot > 0.95f && dot < 1.05f)
	{
		_ship->Movement().MoveTowards(mrs::Vector2Up);
	}
	else if(cross > 0)
	{
		_ship->Movement().MoveTowards(mrs::Vector2Right);
	}
	else if(cross < 0)
	{
		_ship->Movement().MoveTowards(mrs::Vector2Left);
	}
}
