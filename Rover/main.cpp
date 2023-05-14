#include <iostream>

#include <Mars.h>

#include <Core/Input.h>

#include "ImGui/ImGuiLayer.h"

namespace mrs {
	class EditorLayer : public mrs::Layer
	{
	public:
		virtual void OnAttach() override
		{
			// Create renderer
			RendererInitInfo renderer_info = {};
			renderer_info.window = Application::GetInstance().GetWindow();

			// Load main camera
			_main_camera = std::make_shared<Camera>(CameraType::Perspective, renderer_info.window->GetWidth(), renderer_info.window->GetHeight(), glm::vec3(0.0, 0.0, -20.0f));

			renderer_info.camera = _main_camera;

			_renderer = std::make_unique<Renderer>(renderer_info);
			_renderer->Init();
			
			// Load application resources (i.e meshes, textures, materials)
			LoadResources();

			// Instantiate scene world
			_scene = std::make_shared<Scene>();
 
			int s = 20;
			for (uint32_t i = 0; i < s; i++) {
				for (uint32_t j = 0; j < s; j++) {
					mrs::Entity e = _scene->Instantiate();
					e.AddComponent<mrs::RenderableObject>(Mesh::Get("default_mesh"), Material::Get("default_material"));
					e.GetComponent<mrs::Transform>().position = {j * 4, i * 4, 0.0f };
				}
			}

			// Upload gpu resources and create pipeline
			_renderer->UploadResources();
			_renderer->InitPipelines();
		}

		virtual void OnUpdate(float dt) override
		{
			// Render stats
			ImGui::Begin("Stats");
			ImGui::Text("Delta time: %0.8f ms", dt * 1000.0f);
			ImGui::End();

			// Process user input
			ProcessInput(dt);

			// Render
			_renderer->Begin(_scene.get());
			_renderer->End();
		}
	private:

		void LoadResources()
		{
			// Load app resources
			//Mesh::LoadFromAsset("Assets/Models/lost_empire.boop_obj", "default_mesh");
			Mesh::LoadFromAsset("Assets/Models/monkey_smooth.boop_obj", "default_mesh");

			Texture::LoadFromAsset("Assets/Models/lost_empire-RGBA.boop_png", "default_texture");
			Material::Create("default_material");
		}

		void ProcessInput(float dt)
		{
			if (Input::IsKeyPressed(SDLK_w)) {
				_main_camera->GetPosition() += glm::vec3(0.0, 0.0, 5) * dt;
			}
			else if (Input::IsKeyPressed(SDLK_s)) {
				_main_camera->GetPosition() -= glm::vec3(0.0, 0.0, 5) * dt;
			}

			if (Input::IsKeyPressed(SDLK_d)) {
				_main_camera->GetPosition() += glm::vec3(5, 0.0, 0) * dt;
			}
			else if (Input::IsKeyPressed(SDLK_a)) {
				_main_camera->GetPosition() -= glm::vec3(5, 0.0, 0) * dt;
			}
		}
	private:
		// Editor camera
		std::shared_ptr<Camera> _main_camera = nullptr;

		// Application Renderer
		std::unique_ptr<Renderer> _renderer = nullptr;

		// Application Scene
		std::shared_ptr<Scene> _scene;
	};


	class Rover : public Application
	{
	public:
		Rover() : Application("Rover: Mars Editor", 1080, 720)
		{
			PushLayer(new ImGuiLayer);
			PushLayer(new EditorLayer);
		}

		~Rover() {};
	};
}

mrs::Application* mrs::CreateApplication()
{
	return new Rover();
}

