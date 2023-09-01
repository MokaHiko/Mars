#include "Spawner.h"

void Spawner::OnUpdate(float dt)
{
	_timer += dt;
	if (_timer > _spawn_rate)
	{
		SpawnUnit();
		_timer = 0;
	}
}

void Spawner::SpawnUnit() {
	static int ctr = 0;
	auto& e = Instantiate("box" + std::to_string(ctr));
	if (ctr > 8) {
		ctr = 0;
	}

	e.GetComponent<mrs::Transform>().position =
		GetComponent<mrs::Transform>().position + glm::vec3(ctr, ctr, 0.0f);
	ctr++;

	e.AddComponent<mrs::RigidBody2D>();
	e.AddComponent<mrs::RenderableObject>();
	e.AddComponent<mrs::Script>().Bind<Unit>();
}
