#include "SceneGraphLayer.h"

#include "Core/Application.h"
#include "Renderer/Model.h"
#include "ECS/Entity.h "

mrs::SceneGraphLayer::SceneGraphLayer(){}

mrs::SceneGraphLayer::~SceneGraphLayer(){}

void mrs::SceneGraphLayer::OnAttach() {}

void mrs::SceneGraphLayer::OnDetatch() {}

void mrs::SceneGraphLayer::OnEnable() 
{
	_scene = Application::Instance().GetScene();
	_scene->Registry()->on_construct<ModelRenderer>().connect<&SceneGraphLayer::OnModelRendererCreated>(this);

	_scene->Registry()->on_construct<Transform>().connect<&SceneGraphLayer::OnTransformCreated>(this);
	_scene->Registry()->on_destroy<Transform>().connect<&SceneGraphLayer::OnTransformDestroyed>(this);
}

void mrs::SceneGraphLayer::OnDisable() 
{
}

void mrs::SceneGraphLayer::OnUpdate(float dt) 
{
	Transform& node = _scene->Root().GetComponent<Transform>();

	RecursiveUpdate(node);
}

void mrs::SceneGraphLayer::OnTransformCreated(entt::basic_registry<entt::entity>&, entt::entity entity)
{
	Entity e(entity, _scene);
	Transform& transform = e.GetComponent<Transform>();
}

void mrs::SceneGraphLayer::OnTransformDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity)
{
	Entity e(entity, _scene);

	Transform& transform = e.GetComponent<Transform>();

	// Set parent directly to root if parent has been destroyed
	if (transform.parent)
	{
		if (transform.parent.HasComponent<Transform>())
		{
			transform.parent.GetComponent<Transform>().RemoveChild(e);
		}
		else
		{
			transform.parent = _scene->Root();
		}
	}

	// Destroy children
	for (uint32_t i = 0; i < transform.children_count; i++)
	{
		_scene->QueueDestroy(transform.children[i]);
	}
}

void mrs::SceneGraphLayer::RecursiveUpdate(Transform& node)
{
	// TODO: Dirty Check
	for(uint32_t i = 0; i < node.children_count; i++)
	{
		Transform& transform = node.children[i].GetComponent<Transform>();
		transform.UpdateModelMatrix();

		RecursiveUpdate(transform);
	}
}

void mrs::SceneGraphLayer::OnModelRendererCreated(entt::basic_registry<entt::entity> &, entt::entity entity) 
{
	Entity model_entity(entity, _scene);
	Transform& model_transform = model_entity.GetComponent<Transform>();

	Ref<Model> model = model_entity.GetComponent<ModelRenderer>().model;

	for (auto& model_mesh : model->_meshes)
	{
		bool serialize = false;
		Entity e = _scene->Instantiate(model_mesh.mesh->_mesh_name, {}, &serialize);

		auto& mesh_renderer = e.AddComponent<MeshRenderer>(model_mesh.mesh, model->_materials[model_mesh.material_index]);
		model_transform.AddChild(e);
	}
}

