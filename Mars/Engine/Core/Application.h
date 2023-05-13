#ifndef APPLICATION_H
#define APPLICATION_H

#pragma once

#include <memory>

#include "Window.h"
#include "Renderer/Renderer.h"
#include "Renderer/Camera.h"

#include "ECS/Scene.h"

#include "Layer.h"


namespace mrs {
	class Application
	{
	public:
		Application(const std::string& app_name = "First App", uint32_t width = 720,  uint32_t height = 480);
		virtual ~Application();

		void Run();
		void OnUpdate();

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);

		// Get static instance of application
		static Application& GetInstance() { return *_instance; }

		// Returns shared ptr to application window
		std::shared_ptr<Window> GetWindow() { return _window; }
	private:
		LayerStack _layer_stack;
	private:
		std::shared_ptr<Window> _window = nullptr;
		static Application* _instance;

		bool _running = false;
		float _dt = 0.0f;
	};

	// To be implemented by client
	extern mrs::Application* CreateApplication();
}


#endif