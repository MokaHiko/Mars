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
			script_component.DestroyScript();
		}
	}

	Scene::Scene(){}

	Entity Scene::Instantiate(const std::string &name, const glm::vec3 &position)
	{
		Entity e{};
		if(!_free_queue.empty())
		{
			e = Entity{ _registry.create(_free_queue.back()._id), this };
			_free_queue.pop_back();
		}
		else
		{
			e = Entity{_registry.create(), this};
		}

		e.AddComponent<Tag>(name);
		e.AddComponent<Transform>();

		// Emit created signal
		_entity_created(e);

		return e;
	}

	void Scene::QueueDestroy(Entity entity)
	{
		// TODO: Properly destroy/free entities
		//_destruction_queue.push_back(entity);
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
		// Fire off signal
        _entity_destroyed(entity);

		// Add to free queue to recycled entity id
		if(entity)
		{
			_registry.destroy(entity);
		}

		_free_queue.push_back(entity);
	}
}