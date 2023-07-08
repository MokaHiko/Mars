#include "NativeScriptingLayer.h"
#include "Core/Application.h"

void mrs::NativeScriptingLayer::OnAttach()
{
	_name = "NativeScriptingLayer";
}

void mrs::NativeScriptingLayer::OnEnable() {
	_scene = Application::GetInstance().GetScene();

	// Instantiate scripts
	{
		auto view = _scene->Registry()->view<Transform, Script>().use<Script>();
		for (auto entity : view) {
			Entity e(entity, _scene);
			Script &script_component = e.GetComponent<Script>();

			if (!script_component.script)
			{
				script_component.script = script_component.InstantiateScript();
				script_component.script->_game_object = e;

				script_component.script->OnCreate();
			}

			script_component.enabled = true;
		}
	}

	// Call Scripts OnStart
	{
		auto view = _scene->Registry()->view<Transform, Script>().use<Script>();
		for (auto entity : view) {
			Entity e(entity, _scene);
			Script &script_component = e.GetComponent<Script>();

			if (script_component.script) {
				script_component.script->OnStart();
			}
			else {
				script_component.script = script_component.InstantiateScript();
				script_component.script->_game_object = e;
				script_component.enabled = true;

				script_component.script->OnCreate();
				script_component.script->OnStart();
			}
		}
	}
}

void mrs::NativeScriptingLayer::OnDisable()
{
	auto view = _scene->Registry()->view<Transform, Script>();
	for (auto entity : view)
	{
		Entity e(entity, _scene);
		Script &script_component = e.GetComponent<Script>();
		script_component.enabled = false;
	}
}

void mrs::NativeScriptingLayer::OnUpdate(float dt)
{
	// Scripts: Update
	auto view = _scene->Registry()->view<Transform, Script>();
	for (auto entity : view)
	{
		Entity e(entity, _scene);
		Script &script_component = e.GetComponent<Script>();

		if (!script_component.enabled)
		{
			continue;
		}

		if (!script_component.script)
		{
			script_component.script = script_component.InstantiateScript();
			script_component.script->_game_object = e;

			script_component.script->OnCreate();
			script_component.script->OnStart();
			continue;
		}

		e.GetComponent<Script>().script->OnUpdate(dt);
	}
}

void mrs::NativeScriptingLayer::OnEvent(Event &event)
{

}
