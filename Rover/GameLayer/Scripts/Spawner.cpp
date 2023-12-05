#include "Spawner.h"
#include "Player.h"

#include "Unit.h"
#include "Physics/Collider.h"

void Spawner::OnStart()
{
}

void Spawner::OnUpdate(float dt) {
	_timer += dt;
	if (_timer > _spawn_rate)
	{
		SpawnUnit();
		_timer = 0;
	}

	auto& transform = GetComponent<mrs::Transform>();

	float v = 20.0f;
	if(mrs::Input::IsKeyPressed(SDLK_w))
	{
		transform.position.y += v * dt;
	}
	if(mrs::Input::IsKeyPressed(SDLK_s))
	{
		transform.position.y -= v * dt;
	}
	if(mrs::Input::IsKeyPressed(SDLK_d))
	{
		transform.position.x += v * dt;
	}
	if(mrs::Input::IsKeyPressed(SDLK_a))
	{
		transform.position.x -= v * dt;
	}

	static float time = 0;
	time += dt;
	transform.rotation.z = mrs::Cos(time) * 180;
	transform.rotation.y = mrs::Sin(time) * 180;
}

void Spawner::SpawnUnit() {
	static int ctr = 0;
	if (ctr >= _max_units)
	{
		return;
	}

	// TODO: Move to unit factory
	auto& e = Instantiate("Unit" + std::to_string(ctr));
	auto& transform = e.GetComponent<mrs::Transform>();
	transform.position = GetComponent<mrs::Transform>().position + GetComponent<mrs::Transform>().down;
	transform.rotation = glm::vec3(90, 0, 0);
	ctr++;

	e.AddComponent<mrs::RigidBody2D>().use_gravity = true;
	auto& mesh_renderer = e.AddComponent<mrs::MeshRenderer>();

	// TODO: Move to physics on mesh collider created callback
	// auto& col = e.AddComponent<mrs::MeshCollider>();
	// col.collider = CreateRef<mrs::SphereCollider>();
	// col.type = mrs::ColliderType::SphereCollider;

	transform.scale *= 0.25f;
	mesh_renderer.SetMesh(mrs::Mesh::Get("sphere"));
	mesh_renderer.SetMaterial(mrs::Material::Get("default"));

	e.AddScript<Unit>();
}
