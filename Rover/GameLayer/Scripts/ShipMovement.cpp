#include "ShipMovement.h"

AnimateShipTurnProcess::AnimateShipTurnProcess(Ship* ship, float target_rotation, float duration)
	:_ship(ship), _target_rotation(target_rotation), _duration(duration) 
{
	// Turn rotation animates the ship around the y axis
	_initial_rotation = ship->_ship_model.GetComponent<mrs::Transform>().rotation.y;
}

void AnimateShipTurnProcess::OnUpdate(float dt) 
{
	auto& model_transform = _ship->_ship_model.GetComponent<mrs::Transform>();
	model_transform.rotation.y = mrs::Lerp(_initial_rotation, _target_rotation, _time_elapsed / _duration);

	if(_time_elapsed >= _duration)
	{
		Succeed();
	}

	_time_elapsed += dt;
}

void ShipMovement::OnStart()
{
	// Get handles
	auto& parent = GetComponent<mrs::Transform>().parent;
	_ship = parent.GetScript<Ship>();

	if (!_ship)
	{
		MRS_INFO("Required ship as direct parent!");
	}

	_thrusters = _ship->_ship_thrusters;
	if (!_thrusters)
	{
		MRS_INFO("Required ship has direct parent!");
	}
}

void ShipMovement::MoveTowards(mrs::Vector2 direction)
{
	const ShipSpecs& specs = _ship->GetComponent<ShipSpecs>();
	mrs::Transform& transform = _ship->GetComponent<mrs::Transform>();
	mrs::RigidBody2D& rb = _ship->GetComponent<mrs::RigidBody2D>();

	float w = -glm::radians(direction.x * specs.turn_speed);
	rb.SetAngularVelocity(w);
	AnimateTurn(w);

	mrs::Vector2 impulse = transform.up * glm::length(direction) * specs.ms;
	rb.AddImpulse(impulse);

	if(glm::length(direction) > 0.0f)
	{
		_movement_state |= ShipMovementState::Accelerating;
	}
	else
	{
		_movement_state = ShipMovementState::Idle;
	}

	// No Speed Limit on Boost
	if(_movement_state & ShipMovementState::Boosting)
	{
		return;
	}

	// Ship Speed Limit
	mrs::Vector2 velocity = rb.GetVelocity();
	float v_magnitude = glm::length(velocity);
	if (v_magnitude > 0)
	{
		if(v_magnitude > specs.max_speed)
		{
			rb.SetVelocity(glm::normalize(velocity) * specs.max_speed);
		}
	}
}

void ShipMovement::Boost(float multiplier, float duration) 
{
	if(_movement_state & ShipMovementState::Boosting)
	{
		MRS_INFO("Already in boost!");
	}
	_movement_state |= ShipMovementState::Boosting;

	ShipSpecs& specs = _ship->GetComponent<ShipSpecs>();
	float start_ms = specs.ms;
	specs.ms *= multiplier;

	auto boost = CreateRef<mrs::DelayProcess>(duration, [&, start_ms](){
		_movement_state &= (ShipMovementState)(~ShipMovementState::Boosting);

		ShipSpecs& specs = _ship->GetComponent<ShipSpecs>();
		specs.ms = start_ms;
	});
	StartProcess(boost);
}

void ShipMovement::OnUpdate(float dt) {

	if(_movement_state == ShipMovementState::Idle)
	{
		_thrusters.GetComponent<mrs::ParticleSystem>().Stop();
	} 

	if(_movement_state & ShipMovementState::Accelerating)
	{
		_thrusters.GetComponent<mrs::ParticleSystem>().Play();
	}
}

void ShipMovement::OnCollisionEnter2D(mrs::Entity other) 
{
}

void ShipMovement::AnimateTurn(float w) 
{
	_dir = mrs::SignOf<float>(w);
	if(_dir != _last_dir)
	{
		if(_ship_turn_animation != nullptr) // Terminate last animation if exists
		{
			_ship_turn_animation->Succeed();
		}

		_ship_turn_animation = CreateRef<AnimateShipTurnProcess>(_ship, 45.0f * _dir, 0.5f);
		StartProcess(_ship_turn_animation);

		_last_dir = _dir;
	}
}
