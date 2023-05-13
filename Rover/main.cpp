#include <iostream>

#include <Mars.h>

#include <Core/Input.h>

namespace mrs {
	class EditorLayer : public mrs::Layer
	{
	public:
		virtual void OnAttach() override
		{
			// Create editor camera
			_main_camera = std::make_shared<Camera>(CameraType::Perspective, 720, 480, glm::vec3(0.0, 0.0, -20.0f));

			// Create renderer
			RendererInitInfo renderer_info = {};
			renderer_info.window = Application::GetInstance().GetWindow();
			renderer_info.camera = _main_camera;

			_renderer = std::make_unique<Renderer>(renderer_info);
			_renderer->Init();

			// Instantiate scene world
			_scene = std::make_shared<Scene>();

			// Load app resources
			auto mesh = Mesh::LoadFromAsset("Assets/Models/lost_empire.boop_obj", "default_mesh");
			Texture::LoadFromAsset("Assets/Models/lost_empire-RGBA.boop_png", "default_texture");
			auto mat = Material::Create("default_material");

			auto monkey = Mesh::LoadFromAsset("Assets/Models/monkey_smooth.boop_obj", "monkey");
			mrs::Entity e = _scene->Instantiate();
			e.AddComponent<mrs::RenderableObject>(mesh, mat);

			// Upload gpu resources then create pipeline
			_renderer->UploadResources();
			_renderer->InitPipelines();
		}

		virtual void OnUpdate(float dt) override
		{
			ProcessInput(dt);

			_renderer->Render(_scene.get());
		}
	private:

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
		Rover() : Application("Rover: Mars Editor", 720, 480)
		{
			// Push editor layer
			PushLayer(new EditorLayer);
		}

		~Rover() {};
	};
}

mrs::Application* mrs::CreateApplication()
{
	return new Rover();
}

