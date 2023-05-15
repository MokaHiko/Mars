#include "Application.h"

#include "Input.h"
#include "Util/Utils.h"

namespace mrs {
	Application* Application::_instance = nullptr;
	Application::Application(const std::string& app_name, uint32_t width, uint32_t height)
	{
		// Singleton app instance
		assert(!_instance);
		_instance = this;

		// Create window
		_window = std::make_shared<Window>(app_name.c_str(), width, height);
	}

	Application::~Application() {}

	void Application::Run()
	{
		while(_running = _window->Update())
		{
			// Calculate delta time
			util::Timer app_timer([&](const util::Timer& timer) {
					_dt = timer.delta_;
				});

			// Updating layer stack
			OnUpdate();
		}
	}

	void Application::OnUpdate()
	{
		for (Layer* layer : _layer_stack) {
			layer->OnUpdate(_dt);
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		_layer_stack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PopLayer(Layer* layer)
	{
		_layer_stack.PopLayer(layer);
		layer->OnDetatch();
	}
}