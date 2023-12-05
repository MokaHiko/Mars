#include "NativeScriptingLayer.h"
#include "Core/Application.h"

#include "Scripting/Process.h"

void mrs::NativeScriptingLayer::OnAttach()
{
	_name = "NativeScriptingLayer";

	_scene = Application::Instance().GetScene();

	// Connect to Script Component signals
	Scene* scene = Application::Instance().GetScene();
	scene->Registry()->on_destroy<Script>().connect<&NativeScriptingLayer::OnScriptDestroyed>(this);
}

void mrs::NativeScriptingLayer::OnEnable() {

	// Instantiate/Register scripts
	{
		auto view = _scene->Registry()->view<Transform, Script>().use<Script>();
		for (auto entity : view)
		{
			Entity e(entity, _scene);
			Script& script_component = e.GetComponent<Script>();

			if (!script_component.script)
			{
				script_component.script = script_component.InstantiateScript();
				script_component.script->_game_object = e;

				script_component.script->OnCreate();
			}

			script_component.enabled = true;
		}
	}

	// Call Scripts on Start
	{
		auto view = _scene->Registry()->view<Transform, Script>().use<Script>();
		for (auto entity : view) {
			Entity e(entity, _scene);
			Script& script_component = e.GetComponent<Script>();

			// Call Scripts OnStart
			if (!script_component.script->_on_start_called)
			{
				script_component.script->OnStart();
				script_component.script->_on_start_called = true;
			}
		}
	}
}

void mrs::NativeScriptingLayer::OnDisable()
{
	// Destroy scripts
	auto view = _scene->Registry()->view<Transform, Script>();
	for (auto entity : view)
	{
		Entity e(entity, _scene);
		Script& script_component = e.GetComponent<Script>();
		script_component.enabled = false;
		script_component.DestroyScript();
	}
}

void mrs::NativeScriptingLayer::OnUpdate(float dt)
{
	auto view = _scene->Registry()->view<Transform, Script>();

	for (auto entity : view)
	{
		Entity e(entity, _scene);
		Script& script_component = e.GetComponent<Script>();

		if (!script_component.enabled)
		{
			continue;
		}

		if (!script_component.script->_on_start_called)
		{
			script_component.script->OnStart();
			script_component.script->_on_start_called = true;
		}

		script_component.script->OnUpdate(dt);
	}
}

void mrs::NativeScriptingLayer::OnEvent(Event& event)
{
}

void mrs::NativeScriptingLayer::DisableScripts(Entity except)
{
	auto view = _scene->Registry()->view<Transform, Script>();
	for (auto entity : view)
	{
		Entity e(entity, _scene);

		if (e == except)
		{
			continue;
		}

		Script& script_component = e.GetComponent<Script>();
		script_component.enabled = false;
	}
}

void mrs::NativeScriptingLayer::EnableScripts(Entity except)
{
	auto view = _scene->Registry()->view<Transform, Script>();

	for (auto entity : view)
	{
		Entity e(entity, _scene);
		Script& script_component = e.GetComponent<Script>();

		script_component.enabled = true;
	}
}

void mrs::NativeScriptingLayer::OnScriptBound(Entity entity, Script* script_component)
{
	if (_enabled)
	{
		script_component->script = script_component->InstantiateScript();
		script_component->script->_game_object = entity;
		script_component->enabled = true;

		script_component->script->OnCreate();
	}
}

void mrs::NativeScriptingLayer::OnScriptDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity)
{
	Entity e{ entity, _scene };
	if (e.HasComponent<Script>())
	{
		auto& script_component = e.GetComponent<Script>();

		// Stop all ongoing processes 
		for(auto& process : script_component.script->_processes)
		{
			if(process->IsAlive())
			{
				process->Fail();
			}
		}

		script_component.DestroyScript();
	}
}
