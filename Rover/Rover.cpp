#include <iostream>

#include <Mars.h>
#include <Core/Input.h>
#include <glm/gtc/type_ptr.hpp>

#include <Physics/Physics2DLayer.h>
#include <imgui.h>

#include "Scripts/EditorManager.h"

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
			Scene *_scene = Application::GetInstance().GetScene();

			// [UI]
			ImGui::Begin("Application Stats");
			ImGui::Text("Delta time: %0.3f ms", _dt * 1000.0f);
			ImGui::Text("Mouse Input x: %0.2f , y : %0.2f", Input::GetAxis('x'), Input::GetAxis('y'));

			int id = 0;
			for (auto entity : _scene->Registry()->view<Tag, Transform>())
			{
				ImGui::PushID(id++);

				Entity e = { entity, _scene };
				Tag &tag = e.GetComponent<Tag>();
				Transform &transform = e.GetComponent<Transform>();

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
			Material::Create("green_material", "green");

			// Default meshes
			Mesh::LoadFromAsset("Assets/Models/plane.boop_obj", "plane");
			Mesh::LoadFromAsset("Assets/Models/monkey_smooth.boop_obj", "monkey");
			Mesh::LoadFromAsset("Assets/Models/quad.boop_obj", "quad");

			// Create default shapes

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

			auto &cube = Mesh::Create("cube");
			cube->_vertices = vertices;
			cube->_vertex_count = vertices.size();
			cube->_indices = indices;
			cube->_index_count = indices.size();
		}

		void ProcessInput(float dt)
		{
		}

		void InitObjects()
		{
			Application::GetInstance().GetScene()->Instantiate("Editor manager").AddComponent<Script>().Bind<EditorManager>();
		}

	private:
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

mrs::Application *mrs::CreateApplication()
{
	return MRS_NEW Rover();
}

