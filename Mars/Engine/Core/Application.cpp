#include "Application.h"

namespace mrs {
	Application::Application(const std::string& app_name, uint32_t width, uint32_t height)
	{
		_window = std::make_shared<Window>("First App", width, height);

		// TODO: Make Configurable
		_renderer = std::make_unique<Renderer>(_window);
		_renderer->Init();

		_running = true;
	}


	Application::~Application()
	{
		_renderer->Shutdown();
	}

	void Application::Run()
	{
		// Load app resourcesa
		LoadResources();

		_renderer->UploadResources();
		while (_running)
		{
			_running = _window->Update();
			_renderer->Update();
		}
	}
}