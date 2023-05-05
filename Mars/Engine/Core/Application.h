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
		Application();
		virtual ~Application();

	private:
		void Run();

	private:
		std::shared_ptr<Window> _window = nullptr;
		std::unique_ptr<Renderer> _renderer = nullptr;

		bool _running = false;
	};

	 extern mrs::Application* CreateApplication();
}

// To be implemented by client

#endif