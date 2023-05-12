#ifndef APPLICATION_H
#define APPLICATION_H

#pragma once

#include <memory>

#include "Window.h"
#include "Renderer/Renderer.h"
#include "Renderer/Camera.h"

#include "ECS/Scene.h"


namespace mrs {
	class Application
	{
	public:
		Application(const std::string& app_name = "First App", uint32_t width = 720,  uint32_t height = 480);
		virtual ~Application();

		void Run();
		inline std::shared_ptr<Scene> GetScene() { return _scene; }
	public:

		// Where shaders, textures and meshes are loaded
		virtual void LoadResources() {};

		virtual void OnStart() {};
		virtual void OnUpdate() {};
	public:

		inline const float GetDeltaTime() const { return _dt; }

	protected:

		// Editor camera
		std::shared_ptr<Camera> _main_camera = nullptr;
	private:
		std::shared_ptr<Window> _window = nullptr;
		std::unique_ptr<Renderer> _renderer = nullptr;

		bool _running = false;
		float _dt = 0.0f;
	private:
		std::shared_ptr<Scene> _scene;
	};

	// To be implemented by client
	extern mrs::Application* CreateApplication();
}


#endif