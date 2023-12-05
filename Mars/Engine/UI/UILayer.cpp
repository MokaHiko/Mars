#include "UILayer.h"
#include "Core/Application.h"

#include "UI/UI.h"

mrs::UILayer::UILayer() 
	:Layer("UILayer") {}

mrs::UILayer::~UILayer() {}

void mrs::UILayer::OnEnable() 
{
	_scene = Application::Instance().GetScene();
}

void mrs::UILayer::OnDisable() 
{
	_scene = nullptr;
}

void mrs::UILayer::OnUpdate(float dt) 
{
	// Generated vertices/meshes for text
	auto view = _scene->Registry()->view<Transform, Text>();

	for(auto& entity : view)
	{
		Entity e(entity, _scene);
		auto& text_component = e.GetComponent<mrs::Text>();

		for(const char c : text_component.text)
		{
		}
	}
}
