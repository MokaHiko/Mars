#include <iostream>

#include <Mars.h>

#include <Core/Input.h>

#include <glm/gtc/type_ptr.hpp>
#include "Physics/Physics2DLayer.h"

#include "imgui.h"

namespace mrs {
	class EditorLayer : public Layer
	{
	public:
		virtual void OnAttach() override
		{
			LoadEditorResources();
			InitObjects();
		}

		virtual void OnDetatch() override
		{

		}

		virtual void OnUpdate(float dt) override
		{
			_dt = dt;
			ProcessInput(dt);
		}

		virtual void OnImGuiRender() override
		{
			// Scene
			Scene* _scene = Application::GetInstance().GetScene();

			// [UI]
			ImGui::Begin("Application Stats");
			ImGui::Text("Delta time: %0.8f ms", _dt * 1000.0f);
			ImGui::Text("Penis");

			int id = 0;
			for (auto entity : _scene->Registry()->view<Transform, RigidBody2D>()) 
			{
				ImGui::PushID(id++);

				Entity e = { entity, _scene};
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
		}

	private:
		void LoadEditorResources()
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
			std::vector<Vertex> vertices = {
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

			auto window = Application::GetInstance().GetWindow();
		}

		void ProcessInput(float dt)
		{
			static float movement_speed = 40.0f;

			Camera& cam = _main_camera.GetComponent<Camera>();

			if (Input::IsKeyPressed(SDLK_SPACE)) {
				cam.GetPosition() += cam.GetUp() * movement_speed * dt;
			}
			else if (Input::IsKeyPressed(SDLK_q)) {
				cam.GetPosition() -= cam.GetUp() * movement_speed * dt;
			}

			if (Input::IsKeyPressed(SDLK_w)) {
				cam.GetPosition() += cam.GetFront() * movement_speed * dt;
			}
			else if (Input::IsKeyPressed(SDLK_s)) {
				cam.GetPosition() -= cam.GetFront() * movement_speed * dt;
			}

			if (Input::IsKeyPressed(SDLK_d)) {
				cam.GetPosition() += cam.GetRight() * movement_speed * dt;
			}
			else if (Input::IsKeyPressed(SDLK_a)) {
				cam.GetPosition() -= cam.GetRight() * movement_speed * dt;
			}
		}
	
		void InitObjects()
		{
			float z_offset = 1000.0f;

			// Scene
			Scene* _scene = Application::GetInstance().GetScene();

			// Camera
			auto window = Application::GetInstance().GetWindow();
			_main_camera = _scene->Instantiate("Editor Camera");
			_main_camera.AddComponent<Camera>(CameraType::Perspective, window->GetWidth(), window->GetHeight());

			// Light source
			_directional_light = _scene->Instantiate("Directional Light");
			_directional_light.AddComponent<RenderableObject>(Mesh::Get("monkey"), Material::Get("default_material"));
			_directional_light.AddComponent<DirectionalLight>();
			_directional_light.GetComponent<Transform>().position = { 150, 900, z_offset};

			// Bawlz
			int s = 10;
			float spacing = 2.0f;
			srand(time(0));
			for (int i = 0; i < s; i++) {
				Entity e = _scene->Instantiate("Ball");
				e.GetComponent<Transform>().position = { i * spacing, i * spacing + 10.0f, z_offset};
				e.AddComponent<RenderableObject>(Mesh::Get("sphere"), Material::Get("default_material"));

				e.AddComponent<RigidBody2D>();
				e.AddComponent<BoxCollider2D>();
			}

			// Crate
			{
				//_test_object = _scene->Instantiate("Container");
				//_test_object.AddComponent<RenderableObject>(Mesh::Get("container"), Material::Get("container"));

				//_test_object.AddComponent<RigidBody2D>();
				//_test_object.AddComponent<BoxCollider2D>();

				//_test_object.GetComponent<Transform>().position = { 0, 10, z_offset};
				//_test_object.GetComponent<Transform>().scale = glm::vec3{ 0.1f };
			}

			// Floor
			{
				Entity stage = _scene->Instantiate("Stage");
				stage.GetComponent<Transform>().position = glm::vec3{ 0.0f, -10.0f, z_offset };
				stage.GetComponent<Transform>().scale = glm::vec3{ 1000.0f, 1.0f, 1.0f };
				stage.AddComponent<RenderableObject>(Mesh::Get("plane"), Material::Get("default_material"));

				stage.AddComponent<RigidBody2D>().type = BodyType::STATIC;
				stage.AddComponent<BoxCollider2D>();
			}
		}
	private:
		// Editor camera
		Entity _main_camera;

		float _mouse_sensitivity = 0.5f;
		bool _constrain_pitch = true;
		float _movement_speed = 50.0f;

		// Lights
		Entity _directional_light = {};
		Entity _test_object = {};

		// Stats
		float _dt = 0.0f;
	};

	class Rover : public Application
	{
	public:
		Rover() : Application("Rover: Mars Editor", 1920, 1080)
		{
			PushLayer(new EditorLayer);
			PushLayer(new mrs::Physics2DLayer);
		}

		~Rover() {};
	};
}

mrs::Application* mrs::CreateApplication()
{
	return new Rover();
}

