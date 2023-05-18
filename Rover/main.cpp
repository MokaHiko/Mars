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
			_directional_light = _scene->Instantiate();
			_directional_light.AddComponent<mrs::RenderableObject>(Mesh::Get("monkey"), Material::Get("default_material"));
			_directional_light.AddComponent<mrs::DirectionalLight>();
			_directional_light.GetComponent<mrs::Transform>().position = { 0, 0, 15.0f };

			// Crate
			int s = 2;
			float spacing = 180.0f;
			for (int i = 0; i < s; i++) {
				for (int j = 0; j < s; j++) {
					mrs::Entity e = _scene->Instantiate();
					e.AddComponent<mrs::RenderableObject>(Mesh::Get("default_mesh"), Material::Get("default_material"));
					e.AddComponent<mrs::Rigidbody>();
					e.GetComponent<mrs::Transform>().position = { j * spacing, i * spacing + 50, 0.0f };
					e.GetComponent<mrs::Transform>().scale = { 0.25f, 0.25f, 0.25f };
				}
			}

			mrs::Entity e = _scene->Instantiate();
			e.AddComponent<mrs::RenderableObject>(Mesh::Get("minecraft_world"), Material::Get("minecraft_world"));


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

			// Inspector
			Transform& dir_transform = _directional_light.GetComponent<Transform>();
			ImGui::Begin("Application Stats");
			ImGui::DragFloat3("Position: ", &dir_transform.position.x);
			ImGui::End();

			// Process user input
			ProcessInput(dt);

			// Game Logic
			auto rbs = _scene->Registry()->view<mrs::Transform, mrs::Rigidbody>();
			for (auto entity : rbs) {
				Entity e = { entity, _scene.get()};
				e.GetComponent<Transform>().rotation.z = glm::sin(glm::radians((float)(SDL_GetTicks() * 0.001)));
			}

			// Render
			_renderer->Begin(_scene.get());
			_renderer->End();
		}
	private:

		void LoadResources()
		{
			// Load app resources
			Mesh::LoadFromAsset("Assets/Models/container.boop_obj", "default_mesh");
			Texture::LoadFromAsset("Assets/Models/textures_container/Container_DiffuseMap.boop_jpg", "default_texture");
			Material::Create("default_material");

			Mesh::LoadFromAsset("Assets/Models/lost_empire.boop_obj", "minecraft_world");
			Texture::LoadFromAsset("Assets/Models/lost_empire-RGBA.boop_png", "minecraft_world_texture");
			Material::Create("minecraft_world", "minecraft_world_texture");

			Mesh::LoadFromAsset("Assets/Models/monkey_smooth.boop_obj", "monkey");


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
			float movement_speed = 100.0f;

			if (Input::IsKeyPressed(SDLK_SPACE)) {
				_main_camera->GetPosition() += glm::vec3(0.0, movement_speed, 0.0) * dt;
			} 
			else if (Input::IsKeyPressed(SDLK_q)) {
				_main_camera->GetPosition() -= glm::vec3(0.0, movement_speed, 0.0) * dt;
			}

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

		// Lights
		Entity _directional_light = {};
	};


	class Rover : public Application
	{
	public:
		Rover() : Application("Rover: Mars Editor", 1920, 1080)
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

