#include "Scene.h"
#include "Entity.h"

#include "Components/Components.h"

namespace mrs
{
	Entity Scene::Instantiate(const std::string &name, const glm::vec3 &position)
	{

		Entity e = Entity(_registry.create(), this);

		e.AddComponent<Tag>(name);
		e.AddComponent<Transform>();

		return e;
	}

	void Scene::QueueDestroy(Entity entity)
	{
        _destruction_queue.push_back(entity);
	}

	void Scene::Destroy(Entity entity)
	{

	}
}