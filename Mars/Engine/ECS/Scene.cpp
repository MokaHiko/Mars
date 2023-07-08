#include "Scene.h"
#include "Entity.h"

#include "Components/Components.h"

namespace mrs
{
	Scene::~Scene()
	{
		auto view = Registry()->view<Transform, Script>();

		// Destroy scripts
		for (auto entity : view)
		{
			Entity e(entity, this);
			Script &script_component = e.GetComponent<Script>();
			script_component.script = script_component.InstantiateScript();
			script_component.DestroyScript(script_component.script);
		}
	}

	Scene::Scene(){}

	Entity Scene::Instantiate(const std::string &name, const glm::vec3 &position)
	{
		Entity e;
		if (!_free_queue.empty())
		{
			e = _free_queue.back();
			_free_queue.pop_back();
		}
		else
		{
			e = Entity(_registry.create(), this);
		}

		e.AddComponent<Tag>().tag = name;
		e.AddComponent<Transform>();

		return e;
	}

	void Scene::QueueDestroy(Entity entity)
	{
		_destruction_queue.push_back(entity);
	}

	void Scene::FlushDestructionQueue()
	{
		for (Entity e : _destruction_queue)
		{
			Destroy(e);
		};

		if (!_destruction_queue.empty())
		{
			_destruction_queue.clear();
		}
	}

	void Scene::Destroy(Entity entity)
	{
		// Remove all components from entity
		for (auto [id, storage] : _registry.storage())
		{
			storage.remove(entity._id);
		}

		// Add to free to be recycled
		_free_queue.push_back(entity);
	}
}