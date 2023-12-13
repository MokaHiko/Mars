#include "Spawner.h"
#include "Core/Time.h"

void mrs::Spawner::OnStart()
{
	AddComponent<MeshRenderer>().SetMesh(Mesh::Get("cube"));
}

void mrs::Spawner::OnUpdate(float dt) 
{
	static float time = 0.0f;
	static float time_elapsed = 0.0f;
	static float duration = 5.0f;
	time += Time::DeltaTime();
	time_elapsed += Time::DeltaTime();

	static Vector3 p1 = { -10.0f, 0.0f,  30.0f };
	static Vector3 p2 = { 10.0f,  0.0f,  30.0f };

	auto& transform = GetComponent<mrs::Transform>();
	transform.position = mrs::Lerp(p1, p2, time_elapsed / 5.0f);
	transform.rotation = transform.position;

	if (time_elapsed >= 5.0f) {
		p1.x *= -1;
		p2.x *= -1;

		time_elapsed = 0.0f;
	}

	static int ctr = 0;
	if(ctr > 10)
	{
		return;
	}
	if (time >= 0.5f) {
		ctr++;
		auto e = Instantiate("sphere", { transform.position.x, transform.position.y - 1.0f, 30.0f });
		auto& new_pos = e.GetComponent<mrs::Transform>().position;

		if(ctr % 2)
		{
			e.AddComponent<mrs::MeshRenderer>().SetMesh(Mesh::Get("sphere"));
		}
		else
		{
			e.AddComponent<mrs::MeshRenderer>(Mesh::Get("sphere"), Material::Get("blue"));
		}
		e.AddScript<Unit>();

		time = 0.0f;
	}
}

void mrs::Unit::OnUpdate(float dt) 
{
	time_elapsed += dt;
	GetComponent<mrs::Transform>().position.y += dir * dt * 10.0f;
	if(time_elapsed > 0.5f)
	{
		time_elapsed = 0.0f;
		dir *= -1.0f;
	}
}
