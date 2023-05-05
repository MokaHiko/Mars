#include "Application.h"

namespace mrs {
	Application::Application()
	{
		_window = std::make_shared<Window>("First App", 720, 480);

		// TODO: Make Configurable
		_renderer = std::make_unique<Renderer>(_window);
		_renderer->Init();

		_running = true;
		Run();
	}

	Application::~Application()
	{
		_renderer->Shutdown();

	}
	void Application::Run()
	{
		while (_running)
		{
			_running = _window->Update();
			_renderer->Update();
		}
	}
}