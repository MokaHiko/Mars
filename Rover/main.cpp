#include <iostream>

#include <Mars.h>

#include <Core/Input.h>

#include "ImGui/ImGuiLayer.h"
#include <glm/gtc/type_ptr.hpp>

namespace mrs {
	class EditorLayer : public mrs::Layer
	{
	public:
		virtual void OnAttach() override
		{
			// Create renderer
			InitRenderer();

			// Load application resources (i.e meshes, textures, materials)
			LoadResources();

			// Instantiate scene world
			_scene = std::make_shared<Scene>();

			// Light source
			_directional_light = _scene->Instantiate("Directional Light");
			_directional_light.AddComponent<mrs::RenderableObject>(Mesh::Get("monkey"), Material::Get("default_material"));
			_directional_light.AddComponent<mrs::DirectionalLight>();
			_directional_light.GetComponent<mrs::Transform>().position = { 150, 900, -350 };

			// Bawlz
			int s = 5;
			float spacing = 5.0f;
			srand(time(0));
			for (int i = 0; i < s; i++) {
				mrs::Entity e = _scene->Instantiate("Ball");
				e.GetComponent<mrs::Transform>().position = { spacing * spacing, i * spacing + 10.0f, 0.0f };

				e.AddComponent<mrs::RenderableObject>(Mesh::Get("sphere"), Material::Get("white"));

				e.AddComponent<mrs::Rigidbody>().body.use_gravity = true;
				e.AddComponent<mrs::Rigidbody>().body._type = ast::Body::Type::Dynamic;
				e.AddComponent<mrs::Rigidbody>().body.velocity = glm::vec3((rand() % 5) * 2 - 5.0f);

				e.AddComponent<mrs::SphereCollider>();
			}

			_monkey = _scene->Instantiate("Monkey");
			_monkey.AddComponent<mrs::RenderableObject>(Mesh::Get("monkey"), Material::Get("white"));
			_monkey.GetComponent<mrs::Transform>().position = { 0, 5, 0 };
			_monkey.AddComponent<mrs::Rigidbody>().body.use_gravity = true;
			_monkey.AddComponent<mrs::Rigidbody>().body._type = ast::Body::Type::Dynamic;
			_monkey.AddComponent<mrs::SphereCollider>();

			mrs::Entity stage = _scene->Instantiate("Stage");
			stage.GetComponent<mrs::Transform>().scale = { 100, 100.0f, 100.0f };
			stage.AddComponent<mrs::RenderableObject>(Mesh::Get("plane"), Material::Get("white"));

			stage.AddComponent<mrs::Rigidbody>().body.use_gravity = false;
			stage.AddComponent<mrs::Rigidbody>().body._type = ast::Body::Type::Static;
			stage.AddComponent<mrs::PlaneCollider>();

			InitPhysics();

			// Upload gpu resources and create pipeline
			_renderer->UploadResources();
			_renderer->InitPipelines();
		}

		virtual void OnDetatch() override
		{
			_renderer->Shutdown();
			_physics_world->Shutdown();
		}

		virtual void OnUpdate(float dt) override
		{
			// Process user input
			ProcessInput(dt);

			// Update Physics
			_physics_world->Step(dt);

			// [Profiling]
			ImGui::Begin("Application Stats");

			ImGui::Text("Delta time: %0.8f ms", dt * 1000.0f);

			// Inspector
			int id = 0;
			for (auto entity : _scene->Registry()->view<Transform>()) {
				ImGui::PushID(id++);

				Entity e = { entity, _scene.get() };
				Tag& tag = e.GetComponent<Tag>();
				Transform& transform = e.GetComponent<Transform>();

				ImGui::Text(tag.tag.c_str());
				ImGui::Text("Position: ");  ImGui::SameLine();
				ImGui::DragFloat3("##Position", glm::value_ptr(transform.position));
				ImGui::PopID();
			}


			ImGui::End();

			// Render
			_renderer->Begin(_scene.get());
			_renderer->End();
		}
	private:
		void LoadResources()
		{
			// Load app resources
			Mesh::LoadFromAsset("Assets/Models/sphere.boop_obj", "sphere");
			Texture::LoadFromAsset("Assets/Models/textures_container/Container_DiffuseMap.boop_jpg", "default_texture");
			Material::Create("default_material");

			//Mesh::LoadFromAsset("Assets/Models/container.boop_obj", "default_mesh");
			//Texture::LoadFromAsset("Assets/Models/textures_container/Container_DiffuseMap.boop_jpg", "default_texture");
			//Material::Create("default_material");

			//Mesh::LoadFromAsset("Assets/Models/lost_empire.boop_obj", "minecraft_world");
			//Texture::LoadFromAsset("Assets/Models/lost_empire-RGBA.boop_png", "minecraft_world_texture");
			//Material::Create("minecraft_world", "minecraft_world_texture");

			Mesh::LoadFromAsset("Assets/Models/plane.boop_obj", "plane");
			Texture::LoadFromAsset("Assets/Models/white.boop_png", "white");
			Material::Create("white", "white");

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
			static float movement_speed = 40.0f;
			ImGui::Begin("Camera Controls", nullptr, ImGuiWindowFlags_NoResize);
			ImGui::Text("Movmement Speed");
			ImGui::DragFloat("##MovementSpeed", &movement_speed);
			ImGui::End();

			if (Input::IsKeyPressed(SDLK_SPACE)) {
				_main_camera->GetPosition() += glm::vec3(0.0, movement_speed, 0.0) * dt;
			}
			else if (Input::IsKeyPressed(SDLK_q)) {
				_main_camera->GetPosition() -= glm::vec3(0.0, movement_speed, 0.0) * dt;
			}

			if (Input::IsKeyPressed(SDLK_w)) {
				_main_camera->GetPosition() -= glm::vec3(0.0, 0.0, movement_speed) * dt;
			}
			else if (Input::IsKeyPressed(SDLK_s)) {
				_main_camera->GetPosition() += glm::vec3(0.0, 0.0, movement_speed) * dt;
			}

			if (Input::IsKeyPressed(SDLK_d)) {
				_main_camera->GetPosition() += glm::vec3(movement_speed, 0.0, 0) * dt;
			}
			else if (Input::IsKeyPressed(SDLK_a)) {
				_main_camera->GetPosition() -= glm::vec3(movement_speed, 0.0, 0) * dt;
			}
		}

		void InitRenderer()
		{
			RendererInitInfo renderer_info = {};
			renderer_info.window = Application::GetInstance().GetWindow();

			// Load main camera
			_main_camera = std::make_shared<Camera>(CameraType::Perspective, renderer_info.window->GetWidth(), renderer_info.window->GetHeight(), glm::vec3(0.0, 0.0, 50.0f));

			renderer_info.camera = _main_camera;

			_renderer = std::make_unique<Renderer>(renderer_info);
			_renderer->Init();
		}

		void InitPhysics()
		{
			_physics_world = std::make_unique<ast::World>();

			ast::WorldSettings physics_settings = {};
			_physics_world->Init(physics_settings);

			auto view = _scene->Registry()->view<Transform, Rigidbody>();
			for (auto entity : view) {
				Entity e(entity, _scene.get());

				// Register body and colliders 
				ast::Body& rb = e.GetComponent<Rigidbody>().body;
				rb.transform = (ast::Transform*)(&(e.GetComponent<Transform>()));

				if (e.HasComponent<SphereCollider>()) {
					rb.collider = &e.GetComponent<SphereCollider>().sphere_collider;
				}

				if (e.HasComponent<PlaneCollider>()) {
					rb.collider = &e.GetComponent<PlaneCollider>().plane_collider;
				}

				_physics_world->AddBody(&rb);
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
		Entity _monkey = {};

		// Physics
		std::unique_ptr<ast::World> _physics_world = nullptr;
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

