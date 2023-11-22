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
			Script& script_component = e.GetComponent<Script>();
			script_component.script = script_component.InstantiateScript();
			script_component.DestroyScript();
		}
	}

	Entity Scene::Root()
	{
		return Entity(_root, this);
	}

	Scene::Scene()
	{
		// Create root node
		Entity e = {};
		if (!_free_queue.empty())
		{
			Entity idx = _free_queue.back();
			_free_queue.pop_back();

			e = Entity{ _registry.create(idx), this };
		}
		else
		{
			e = Entity{ _registry.create(), this };
		}

		// Determine if serializable
		//e.AddComponent<Serializer>().serialize = false;
		e.AddComponent<Tag>("Root");

		// Transform component
		auto& transform = e.AddComponent<Transform>();
		transform.self = e;

		// Root node has no parent
		transform.parent = {};

		// Fire off created signal
		_entity_created(e);
		_root = e._id;
	}

	Entity Scene::Instantiate(const std::string& name, const glm::vec3& position, bool* serialize)
	{
		Entity e = {};
		if (!_free_queue.empty())
		{
			Entity idx = _free_queue.back();
			_free_queue.pop_back();

			e = Entity{ _registry.create(idx), this };
		}
		else
		{
			e = Entity{ _registry.create(), this };
		}

		// Determine if serializable
		e.AddComponent<Serializer>().serialize = serialize != nullptr ? *serialize : _serializing;
		e.AddComponent<Tag>(name);

		// Transform component
		auto& transform = e.AddComponent<Transform>();
		transform.self = e;
		transform.position = position;

		// Add to root if not provided 
		Entity root = { _root, this };
		root.GetComponent<Transform>().AddChild(e);

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
		if (entity)
		{
			if (entity == _root)
			{
				MRS_INFO("Cannot destroy root!");
				return;
			}

			// Fire off destroy signal
			_entity_destroyed(entity);

			// Store last entt
			_free_queue.push_back(entity);

			// Remove all components
			Registry()->destroy(entity);
		}
	}
}