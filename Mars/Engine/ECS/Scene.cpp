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

	Entity Scene::Instantiate(const std::string& name, const glm::vec3& position, bool* serialize)
	{
		Entity e = {};
		if(!_free_queue.empty())
		{
			Entity idx = _free_queue.back();
			_free_queue.pop_back();

			e = Entity{_registry.create(idx), this};
		}
		else
		{
			e = Entity{_registry.create(), this};
		}

		// Determine if serializable
		e.AddComponent<Serializer>().serialize = serialize != nullptr ? *serialize : _serializing;

		e.AddComponent<Tag>(name);
		e.AddComponent<Transform>().position = position;

		// Fire off created signal
		_entity_created(e);
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
		// Add to free queue to recycled entity id
		if(entity)
		{
			// Fire off destroy signal
			_entity_destroyed(entity);

			// Store last entt
			_free_queue.push_back(entity);

			// Remove all components
			Registry()->destroy(entity);

			entity._id = {};
		}
	}
}