#include "Application.h"

#include "Input.h"
#include "Util/Utils.h"

namespace mrs {
	Application::Application(const std::string& app_name, uint32_t width, uint32_t height)
	{
		// Create window
		_window = std::make_shared<Window>("First App", width, height);

		// Create editor camera
		_main_camera = std::make_shared<Camera>(CameraType::Perspective, width, height, glm::vec3(0.0f, 0.0f, -2.0f));

		// Create renderer
		RendererInitInfo renderer_info = {};
		renderer_info.window = _window;
		renderer_info.camera = _main_camera;

		_renderer = std::make_unique<Renderer>(renderer_info);
		_renderer->Init();

		// Instantiate Scene
		_scene = std::make_shared<Scene>();

		_running = true;
	}


	Application::~Application()
	{
		_renderer->Shutdown();
	}

	void Application::Run()
	{
		// Load app resources
		LoadResources();

		// Upload gpu resources then create pipeline
		_renderer->UploadResources();
		_renderer->InitPipelines();

		OnStart();
		while (_running)
		{
			util::Timer app_timer([&](const util::Timer& timer) {
					_dt = timer.delta_;
				});
			_running = _window->Update();

			_renderer->Render(_scene.get());

			OnUpdate();
		}
	}
}