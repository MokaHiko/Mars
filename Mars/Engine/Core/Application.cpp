#include "Application.h"

#include "Core/Log.h"
#include "Core/Input.h"
#include "Core/Time.h"
#include "Toolbox/TimeToolBox.h"

namespace mrs {
	Application *Application::_instance = nullptr;
	Application::Application(const std::string &app_name, uint32_t width, uint32_t height)
	{
		// Initialize single app instance
		assert(!_instance);
		_instance = this;

		// Check system specs
		MRS_INFO("Initializing Application");
		Platform::CheckStorage((size_t)(4 * glm::pow(10, 8)));

		// Create window
		_app_name = app_name;
		_window = std::make_shared<Window>(app_name.c_str(), width, height);

		// Create scene
		_scene = std::make_shared<Scene>();
	}

	Application::~Application() {}

	void Application::Run()
	{
		// Enable layers
		for (Layer *layer : _layer_stack)
		{
			if(!layer->IsEnabled())
			{
				layer->Enable();
			}
		}

		// Set Fixed Delta time
		Time::SetFixedDeltaTime(1/144.0f);

		// Main application loop
		while (_running = PollEvents())
		{
			// Calculate delta time
			tbx::Timer app_timer([&](const tbx::Timer &timer) {
				Time::SetDeltaTime(static_cast<float>(timer.delta_));
			});

			for (Layer *layer : _layer_stack)
			{
				if(layer->IsEnabled())
				{
					layer->OnUpdate(Time::DeltaTime());
				}
			}

			static float fixed_elapsed = 0.0f;
			fixed_elapsed += Time::DeltaTime();
			if(fixed_elapsed >= Time::FixedDeltaTime())
			{
				for (Layer *layer : _layer_stack)
				{
					if(layer->IsEnabled())
					{
						layer->OnFixedUpdate(Time::FixedDeltaTime());
					}
				}
				fixed_elapsed = 0.0f;
			}

			_scene->FlushDestructionQueue();
		}
	}

	void Application::Shutdown()
	{
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

	void Application::DisableLayer(const std::string &layer_name)
	{
		_layer_stack.DisableLayer(layer_name);
	}

	void Application::EnableLayer(const std::string &layer_name)
	{
		_layer_stack.EnableLayer(layer_name);
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