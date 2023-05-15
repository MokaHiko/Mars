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

			// Light source
			mrs::Entity light = _scene->Instantiate();
			light.AddComponent<mrs::RenderableObject>(Mesh::Get("cube"), Material::Get("default_material"));
			light.AddComponent<mrs::DirectionalLight>();
			light.GetComponent<mrs::Transform>().position = { 10, 10, -10.0f };

			// Crate
			int s = 5;
			float spacing = 180.0f;
			for (int i = -s/2; i < s /2; i++) {
				for (int j = -s/2 ; j < s/2; j++) {
					mrs::Entity e = _scene->Instantiate();
					e.AddComponent<mrs::RenderableObject>(Mesh::Get("default_mesh"), Material::Get("default_material"));
					e.GetComponent<mrs::Transform>().position = { j * spacing, i * spacing, -250.0f };
					e.GetComponent<mrs::Transform>().scale = { 0.25f, 0.25f, 0.25f };
				}
			}

			// Upload gpu resources and create pipeline
			_renderer->UploadResources();
			_renderer->InitPipelines();
		}

		virtual void OnUpdate(float dt) override
		{
			// [Profiling]
			ImGui::Begin("Application Stats");
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
			//Texture::LoadFromAsset("Assets/Models/lost_empire-RGBA.boop_png", "default_texture");
			Mesh::LoadFromAsset("Assets/Models/container.boop_obj", "default_mesh");
			Texture::LoadFromAsset("Assets/Models/textures_container/Container_DiffuseMap.boop_jpg", "default_texture");
			Material::Create("default_material");

			// Creatae default shapes

			// ~ Cube
			std::vector<mrs::Vertex> vertices = {
			  {{-1.0f,1.0f,0.0f}, {}, {}, {}},
			  {{-1.0f,-1.0f,0.0f}, {}, {}, {}},
			  {{1.0f,1.0f,0.0f}, {}, {}, {}},
			  {{1.0f,-1.0f,0.0f}, {}, {}, {}},
			  {{-1.0f,1.0f,-1.0f}, {}, {}, {}},
			  {{-1.0f,-1.0f,-1.0f}, {}, {}, {}},
			  {{1.0f,1.0f,-1.0f}, {}, {}, {}},
			  {{1.0f,-1.0f,-1.0f}, {}, {}, {}}
			};

			std::vector<uint32_t> indices = {
				  0, 2, 3, 0, 3, 1,
				  2, 6, 7, 2, 7, 3,
				  6, 4, 5, 6, 5, 7,
				  4, 0, 1, 4, 1, 5,
				  0, 4, 6, 0, 6, 2,
				  1, 5, 7, 1, 7, 3,
			};

			auto& cube = Mesh::Create("cube");
			cube->_vertices = vertices;
			cube->_vertex_count = vertices.size();
			cube->_indices = indices;
			cube->_index_count = indices.size();
		}

		void ProcessInput(float dt)
		{
			float movement_speed = 20.0f;
			if (Input::IsKeyPressed(SDLK_w)) {
				_main_camera->GetPosition() += glm::vec3(0.0, 0.0, movement_speed) * dt;
			}
			else if (Input::IsKeyPressed(SDLK_s)) {
				_main_camera->GetPosition() -= glm::vec3(0.0, 0.0, movement_speed) * dt;
			}

			if (Input::IsKeyPressed(SDLK_d)) {
				_main_camera->GetPosition() += glm::vec3(movement_speed, 0.0, 0) * dt;
			}
			else if (Input::IsKeyPressed(SDLK_a)) {
				_main_camera->GetPosition() -= glm::vec3(movement_speed, 0.0, 0) * dt;
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

