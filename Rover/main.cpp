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
			InitObjects();
			InitPhysics();
			InitTerrain();

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

			// [UI]
			ImGui::Begin("Application Stats");
			ImGui::Text("Delta time: %0.8f ms", dt * 1000.0f);

			int id = 0;
			for (auto entity : _scene->Registry()->view<Transform, Rigidbody>()) {
				ImGui::PushID(id++);

				Entity e = { entity, _scene.get() };
				Tag& tag = e.GetComponent<Tag>();
				Transform& transform = e.GetComponent<Transform>();

				ImGui::Text(tag.tag.c_str());
				ImGui::Text("Position: ");  ImGui::SameLine();
				ImGui::DragFloat3("##Position", glm::value_ptr(transform.position));
				ImGui::Text("Rotation: ");  ImGui::SameLine();
				ImGui::DragFloat3("##Rotation", glm::value_ptr(transform.rotation));
				ImGui::Text("Scale: ");  ImGui::SameLine();
				ImGui::DragFloat3("##Scale", glm::value_ptr(transform.scale));
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
			Texture::LoadFromAsset("Assets/Models/white.boop_png", "default_texture");
			Material::Create("default_material");

			Mesh::LoadFromAsset("Assets/Models/container.boop_obj", "container");
			Texture::LoadFromAsset("Assets/Models/textures_container/Container_DiffuseMap.boop_jpg", "container");
			Material::Create("container", "container");

			Mesh::LoadFromAsset("Assets/Models/grass_blade.boop_obj", "grass");
			Texture::LoadFromAsset("Assets/Textures/green.boop_png", "green");
			Material::Create("green", "green");

			Mesh::LoadFromAsset("Assets/Models/plane.boop_obj", "plane");
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
			glm::vec2 offset = Input::MouseOffset();
			ProcessMouseMovement(offset.x, offset.y);

			static float movement_speed = 40.0f;
			ImGui::Begin("Camera Controls", nullptr, ImGuiWindowFlags_NoResize);
			ImGui::Text("Movmement Speed");
			ImGui::DragFloat("##MovementSpeed", &movement_speed);
			ImGui::End();

			if (Input::IsKeyPressed(SDLK_SPACE)) {
				_main_camera->GetPosition() += _main_camera->GetUp() * movement_speed * dt;
			}
			else if (Input::IsKeyPressed(SDLK_q)) {
				_main_camera->GetPosition() -= _main_camera->GetUp() * movement_speed * dt;
			}

			if (Input::IsKeyPressed(SDLK_w)) {
				_main_camera->GetPosition() += _main_camera->GetFront() * movement_speed * dt;
			}
			else if (Input::IsKeyPressed(SDLK_s)) {
				_main_camera->GetPosition() -= _main_camera->GetFront() * movement_speed * dt;
			}

			if (Input::IsKeyPressed(SDLK_d)) {
				_main_camera->GetPosition() += _main_camera->GetRight() * movement_speed * dt;
			}
			else if (Input::IsKeyPressed(SDLK_a)) {
				_main_camera->GetPosition() -= _main_camera->GetRight() * movement_speed * dt;
			}
		}
		void ProcessMouseMovement(float xoffset, float yoffset)
		{
			if (!Input::IsMouseButtonPressed(SDL_BUTTON_RIGHT)) {
				SDL_SetRelativeMouseMode(SDL_FALSE);
				return;
			} else {
				SDL_SetRelativeMouseMode(SDL_TRUE);
			}

			xoffset *= _mouse_sensitivity;
			yoffset *= _mouse_sensitivity;

			_main_camera->_yaw += xoffset;
			_main_camera->_pitch -= yoffset;

			if (_constrain_pitch)
			{
				if (_main_camera->_pitch > 89.0f)
					_main_camera->_pitch = 89.0f;
				if (_main_camera->_pitch < -89.0f)
					_main_camera->_pitch = -89.0f;
			}
		}

		void InitObjects()
		{
			// Light source
			_directional_light = _scene->Instantiate("Directional Light");
			_directional_light.AddComponent<mrs::RenderableObject>(Mesh::Get("monkey"), Material::Get("default_material"));
			_directional_light.AddComponent<mrs::DirectionalLight>();
			_directional_light.GetComponent<mrs::Transform>().position = { 150, 900, -350 };

			// Bawlz
			int s = 10;
			float spacing = 2.0f;
			srand(time(0));
			for (int i = 0; i < s; i++) {
				mrs::Entity e = _scene->Instantiate("Ball");
				e.GetComponent<mrs::Transform>().position = { i * spacing, i * spacing + 10.0f, 0.0f };

				e.AddComponent<mrs::RenderableObject>(Mesh::Get("sphere"), Material::Get("default_material"));

				e.AddComponent<mrs::Rigidbody>().body.use_gravity = true;
				e.AddComponent<mrs::Rigidbody>().body._type = ast::Body::Type::Dynamic;
				e.AddComponent<mrs::Rigidbody>().body.velocity = glm::vec3((rand() % 5) * 2 - 5.0f);

				e.AddComponent<mrs::SphereCollider>();
			}

			_test_object = _scene->Instantiate("Container");
			_test_object.AddComponent<mrs::RenderableObject>(Mesh::Get("container"), Material::Get("container"));
			_test_object.GetComponent<mrs::Transform>().position = { 0, 10, 0 };
			_test_object.GetComponent<mrs::Transform>().scale = glm::vec3{ 0.1f };


			mrs::Entity stage = _scene->Instantiate("Stage");
			stage.GetComponent<mrs::Transform>().scale = glm::vec3{ 1000.0f };
			stage.AddComponent<mrs::RenderableObject>(Mesh::Get("plane"), Material::Get("default_material"));

			stage.AddComponent<mrs::Rigidbody>().body.use_gravity = false;
			stage.AddComponent<mrs::Rigidbody>().body._type = ast::Body::Type::Static;
			stage.AddComponent<mrs::PlaneCollider>();
		}
		void InitRenderer()
		{
			RendererInitInfo renderer_info = {};
			renderer_info.window = Application::GetInstance().GetWindow();

			// Load main camera
			_main_camera = std::make_shared<Camera>(CameraType::Perspective, renderer_info.window->GetWidth(), renderer_info.window->GetHeight(), glm::vec3(0.0, 25.0, 25.0f));

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
		void InitTerrain()
		{
			// Grass
			int s = 20;
			int spacing = 1.0f;
			for (int i = 0; i < s; i++) {
				for (int j = 0; j < s; j++) {
					auto& grass = _scene->Instantiate("GrassBlade ");
					grass.AddComponent<mrs::RenderableObject>(Mesh::Get("grass"), Material::Get("green"));
					grass.GetComponent<mrs::Transform>().position = glm::vec3(i * spacing + 50, 0.0f, j * spacing);
					grass.GetComponent<mrs::Transform>().scale = glm::vec3(20.0f);
				}
			}
		}
	private:
		// Editor camera
		std::shared_ptr<Camera> _main_camera = nullptr;
		float _mouse_sensitivity = 0.5f;
		bool _constrain_pitch = true;
		float _movement_speed = 50.0f;

		// Application Renderer
		std::unique_ptr<Renderer> _renderer = nullptr;

		// Application Scene
		std::shared_ptr<Scene> _scene;

		// Lights
		Entity _directional_light = {};
		Entity _test_object = {};

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

