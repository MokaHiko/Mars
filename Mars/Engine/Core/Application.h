#ifndef APPLICATION_H
#define APPLICATION_H

#pragma once

#include <memory>

#include "Window.h"
#include "Renderer/Renderer.h"

namespace mrs {
	class Application
	{
	public:
		Application(const std::string& app_name = "First App", uint32_t width = 720,  uint32_t height = 480);
		virtual ~Application();

		void Run();
	public:
		// Where shaders, textures and meshes are loaded
		virtual void LoadResources() {};
	private:
		std::shared_ptr<Window> _window = nullptr;
		std::unique_ptr<Renderer> _renderer = nullptr;

		bool _running = false;
	};

	// To be implemented by client
	extern mrs::Application* CreateApplication();
}


#endif