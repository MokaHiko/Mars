#include "Spawner.h"
#include "Core/Time.h"

void mrs::Spawner::OnStart()
{
	AddComponent<MeshRenderer>();
}

void mrs::Spawner::OnUpdate(float dt)
{
	static float time = 0.0f;
	static float time_elapsed = 0.0f;
	static float duration = 5.0f;
	time += Time::DeltaTime();
	time_elapsed += Time::DeltaTime();

	static Vector3 p1 = { -40.0f, 50.0f,  0.0f };
	static Vector3 p2 = { 40.0f,  50.0f,  0.0f };

	auto& transform = GetComponent<mrs::Transform>();
	transform.position = mrs::Lerp(p1, p2, time_elapsed / 5.0f);
	transform.rotation += transform.position;

	if (time_elapsed >= 5.0f) {
		p1.x *= -1;
		p2.x *= -1;

		time_elapsed = 0.0f;
	}

	static int ctr = 0;
	if (time >= 0.25f) {
		auto e = Instantiate("cube", { transform.position.x, transform.position.y - 5.0f, 0.0f });
		auto& new_pos = e.GetComponent<mrs::Transform>().position;
		e.GetComponent<mrs::Transform>().rotation.z = rand() % 360;

		if (ctr % 2 == 0)
		{
			e.AddComponent<mrs::MeshRenderer>(Mesh::Get("cube"), Material::Get("red"));
		}
		else
		{
			e.AddComponent<mrs::MeshRenderer>(Mesh::Get("cube"), Material::Get("blue"));
		}
		e.AddScript<Unit>();
		e.AddComponent<mrs::RigidBody2D>();

		auto& trail = e.AddComponent<mrs::ParticleSystem>();
		trail.emission_rate = 32;
		trail.max_particles = 64;
		trail.particle_size = 0.15f;
		trail.spread_angle = 35.0f;
		trail.velocity = { 10, 50 };
		trail.color_1 = { 1.0f, 1.0f, 1.0f, 1.000f };
		trail.color_2 = { 0.0f, 0.0f, 0.0f, 0.25f };
		trail.life_time = 0.55f;
		trail.repeating = true;
		trail.world_space = true;

		time = 0.0f;
		ctr++;
	}
}

void mrs::Unit::OnUpdate(float dt)
{
}

void mrs::Unit::OnCollisionEnter2D(mrs::Collision& col)
{
	if (!_delay_destroy)
	{
		_delay_destroy = CreateRef<DelayProcess>(5.0f, [&] {
			QueueDestroy();
			});

		StartProcess(_delay_destroy);
	}
}
