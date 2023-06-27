#include "Application.h"

#include <imgui_impl_sdl2.h>

#include "Core/Log.h"
#include "Core/InputLayer.h"
#include "Core/Input.h"

#include "Toolbox/TimeToolBox.h"
#include "Renderer/RenderPipelineLayers/DefaultRenderPipelineLayer/DefaultRenderPipelineLayer.h"

namespace mrs {
	Application *Application::_instance = nullptr;
	Application::Application(const std::string &app_name, uint32_t width, uint32_t height)
	{
		// Initialize single app instance
		assert(!_instance);
		_instance = this;

		MRS_TRACE("Initializing Application");
		Platform::CheckStorage((size_t)(4 * glm::pow(10, 8)));

		// Create window
		_window = std::make_shared<Window>(app_name.c_str(), width, height);

		// Create scene
		_scene = std::make_shared<Scene>();

	}

	Application::~Application() {}

	void Application::Run()
	{
		// Push default layers
		PushLayer(new InputLayer());
		PushLayer(new DefaultRenderPipelineLayer());

		// Instantiate scripts
		{
			auto view = _scene->Registry()->view<Transform, Script>();
			for (auto entity : view)
			{
				Entity e(entity, _scene.get());
				Script &script_component = e.GetComponent<Script>();
				script_component.script = script_component.InstantiateScript();
				script_component.script->_game_object = e;

				script_component.script->OnStart();
			}
		}

		while (_running = PollEvents())
		{
			// Calculate delta time
			tbx::Timer app_timer([&](const tbx::Timer &timer) {
				_dt = static_cast<float>(timer.delta_);
				});

			// TODO: Handle inconsistent delta time in a fixed update
			static float dt = 1/144.0f;

			// Scripts: Update
			auto view = _scene->Registry()->view<Transform, Script>();
			for (auto entity : view)
			{
				Entity e(entity, _scene.get());
				Script &script_component = e.GetComponent<Script>();
				if (!script_component.script) {
					script_component.script = script_component.InstantiateScript();
					script_component.script->_game_object = e;

					script_component.script->OnStart();
					continue;
				}

				e.GetComponent<Script>().script->OnUpdate(dt);
			}

			for (Layer *layer : _layer_stack)
			{
				layer->OnUpdate(dt);
			}
		}
	}

	bool Application::PollEvents()
	{
		SDL_Event event;
		Input::Reset();
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				return false;
			}

			ImGui_ImplSDL2_ProcessEvent(&event);
			Event e(event);

			// Propogate event through layers
			for (Layer *layer : _layer_stack)
			{
				layer->OnEvent(e);

				// Escape loop on event handled
				if (e.IsHandled())
				{
					break;
				}
			}
		}

		return true;
	}

	void Application::PushLayer(Layer *layer)
	{
		_layer_stack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PopLayer(Layer *layer)
	{
		_layer_stack.PopLayer(layer);
		layer->OnDetatch();
	}

	Layer *Application::FindLayer(const std::string &layer_name)
	{
		for (Layer *layer : _layer_stack)
		{
			if (layer->GetName() == layer_name)
			{
				return layer;
			}
		}

		MRS_ERROR("Layer not found");
		return nullptr;
	}
}