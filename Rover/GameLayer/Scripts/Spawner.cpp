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
	transform.position = GetComponent<mrs::Transform>().position;
	transform.rotation = glm::vec3(90, 0, 0);
	ctr++;

	e.AddComponent<mrs::RigidBody2D>().use_gravity = false;

	// TODO: Move to physics on mesh collider created callback
	auto& col = e.AddComponent<mrs::MeshCollider>();
	col.collider = CreateRef<mrs::SphereCollider>();
	col.type = mrs::ColliderType::SphereCollider;

	auto& mesh_renderer = e.AddComponent<mrs::MeshRenderer>();

	if(ctr % 2 == 0)	
	{
		transform.scale *= 2.0f;
		mesh_renderer.SetMesh(mrs::Mesh::Get("soldier"));
	}
	else
	{
		mesh_renderer.SetMesh(mrs::Mesh::Get("sphere"));
	}
	mesh_renderer.SetMaterial(mrs::Material::Get("green"));

	e.AddComponent<mrs::Script>().Bind<Unit>();

}
