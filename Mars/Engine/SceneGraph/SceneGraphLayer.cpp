#include "SceneGraphLayer.h"

#include "Core/Application.h"

mrs::SceneGraphLayer::SceneGraphLayer(){}

mrs::SceneGraphLayer::~SceneGraphLayer(){}

void mrs::SceneGraphLayer::OnAttach() {}

void mrs::SceneGraphLayer::OnDetatch() {}

void mrs::SceneGraphLayer::OnEnable() 
{
	_scene = Application::Instance().GetScene();
}

void mrs::SceneGraphLayer::OnDisable() 
{
}

void mrs::SceneGraphLayer::OnUpdate(float dt) 
{
	auto view = _scene->Registry()->view<Tag, Transform>();

	for(auto& entity : view)
	{
		Entity e(entity, _scene);
		e.GetComponent<Transform>().UpdateModelMatrix();
	}
}
