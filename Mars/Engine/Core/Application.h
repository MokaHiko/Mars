#ifndef APPLICATION_H
#define APPLICATION_H

#pragma once

#include <Core/Memory.h>

#include "Window.h"
#include "Renderer/Renderer.h"
#include "Renderer/Camera.h"

#include "ECS/Entity.h"
#include "ECS/Scene.h"
#include "ECS/Components/Components.h"

#include "Layer.h"

namespace mrs {

	struct ApplicationRequirements
	{
		size_t virtual_memory = 0;
		size_t physical_memory = 0;
	};

	// Base class to be inherited by Mars applications
	class Application
	{
	public:
		Application(const std::string& app_name = "First App", uint32_t width = 720, uint32_t height = 480);
		virtual ~Application();

		void Run();
		void Shutdown();

		bool PollEvents();

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);

		void DisableLayer(const std::string& layer_name);
		void EnableLayer(const std::string& layer_name);

		// Returns handle to layer given name
		Layer* FindLayer(const std::string& layer_name);

		// Returns handle to application scene
		Scene* GetScene() { return _scene.get(); }

		// Returns const reference to app name string
		const std::string& GetAppName() const { return _app_name; }

		// Get static instance of application
		static Application& Instance() { return *_instance; }

		// Returns layer stack
		LayerStack& GetLayers() { return _layer_stack; }

		// Returns shared ptr to application window
		Ref<Window> GetWindow() { return _window; }
	private:
		Ref<Window> _window = nullptr;
		Ref<Scene> _scene = nullptr;
		std::string _app_name;

		bool _running = false;
		LayerStack _layer_stack;

		static Application* _instance;
	};

	// To be implemented by client
	extern mrs::Application* CreateApplication();
}
#endif