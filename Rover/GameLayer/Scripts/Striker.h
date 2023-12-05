#ifndef STRIKER_H
#define STRIKER_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

#include "Ship.h"

class AIState
{
public:
	AIState() = default;
	virtual ~AIState() = default;
	virtual void OnBegin() {};
	virtual void OnEnd() {};

	virtual void Update() = 0;
private:
	uint16_t _state_id = -1;
};

enum class EnemyState : uint16_t
{
	Idle, 
	Patrol,
	Follow,
	Run,
};

class StrikerIdleState;
class StrikerPatrolState;
class Striker: public mrs::ScriptableEntity
{
public:
    virtual void OnStart() override;
    virtual void OnCreate() override;
    virtual void OnUpdate(float dt) override;

    Ship* _ship = nullptr;
private:
	void Think();
	void SetState(Ref<AIState> new_state);
private:
	// States
	Ref<StrikerIdleState> _idle_state = nullptr;
	Ref<StrikerPatrolState> _patrol_state = nullptr;

	float _since_last_think = 0.0f;
	float _tick_rate = 1.0f;
	Ref<AIState> _state = nullptr;
};

class StrikerState : public AIState
{
public:
	StrikerState(Striker* striker)
		:_striker(striker)
	{
		_ship = _striker->_ship;
		if(!_ship)
		{
			MRS_INFO("Striker has no ship!");
		}
	};
	virtual ~StrikerState(){};
protected:
	Ship* _ship = nullptr;
	Striker* _striker = nullptr;
};

class StrikerIdleState : public StrikerState
{
public:
	StrikerIdleState(Striker* striker);
	virtual ~StrikerIdleState() = default;
	virtual void OnBegin() override;
	virtual void OnEnd() override;

	virtual void Update() override;
private:
	uint16_t _state_id = -1;
};

class StrikerPatrolState : public StrikerState
{
public:
	StrikerPatrolState(Striker* striker);
	virtual ~StrikerPatrolState() = default;

	virtual void OnBegin() override;
	virtual void OnEnd() override;

	virtual void Update() override;
private:
	float _patrol_range = 100.0f;
	float _time_elapsed = 0.0f;

	mrs::Vector2 _start_pos = {};

	int _current_patrol_point = 0;
	std::vector<mrs::Vector2> _patrol_points = {};
private:
	uint16_t _state_id = -1;
};



#endif