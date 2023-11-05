#include "Ship.h"
#include <Core/Input.h>
#include <Physics/Physics.h>

Ship::Ship()
{

}

Ship::~Ship()
{
}

void Ship::OnCreate() {}

void Ship::OnStart() {
	auto sale_process = CreateRef<mrs::DelayProcess>(2, [](){
		MRS_INFO("Ship sale!");
	});
	auto explode_process = CreateRef<mrs::DelayProcess>(1, [](){
		MRS_INFO("Explode!");
	});

	sale_process->AttachChild(explode_process);

	StartProcess(sale_process);
}

void Ship::OnUpdate(float dt)
{
	// TODO: Move to Ship Controller
	mrs::Transform& transform = GetComponent<mrs::Transform>();
	mrs::RigidBody2D& rb = GetComponent<mrs::RigidBody2D>();

	static float ms = 2.5f;
	static float total_time = 0;
	static float rotation_duration = 1.0f;
	static float target_rotation = 45.0f;

	if (mrs::Input::IsKeyPressed(SDLK_SPACE))
	{
	}

	else if (mrs::Input::IsKeyPressed(SDLK_q))
	{
	}

	if (mrs::Input::IsKeyPressed(SDLK_w))
	{
		rb.AddImpulse({0, ms});
	}
	else if (mrs::Input::IsKeyPressed(SDLK_s))
	{
		rb.AddImpulse({0, -ms});
	}

	if (mrs::Input::IsKeyPressed(SDLK_d))
	{
		rb.AddImpulse({ms, 0});

		total_time += dt;
		transform.rotation.y = Lerp(transform.rotation.y, target_rotation, total_time / rotation_duration);

		if(target_rotation - transform.rotation.y < 0.01f)
		{
			total_time = 0;
		}
	}
	else if (mrs::Input::IsKeyPressed(SDLK_a))
	{
		rb.AddImpulse({-ms, 0});

		total_time += dt;
		transform.rotation.y = Lerp(transform.rotation.y, -target_rotation, total_time / rotation_duration);

		if(-target_rotation - transform.rotation.y < 0.01f)
		{
			total_time = 0;
		}
	}
}

void Ship::OnCollisionEnter2D(mrs::Entity other) {}
