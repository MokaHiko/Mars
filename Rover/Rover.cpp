#include <iostream>

#include <Mars.h>
#include <Core/Input.h>

#include <Physics/Physics2DLayer.h>

#include "Scripts/Editor/EditorManager.h"
#include "UIHelpers.h"

#include "Panels/Menus/MainMenu.h"
#include "Panels/InspectorPanel.h"

#include "Core/InputLayer.h"
#include "Scripting/NativeScriptingLayer.h"
#include "Renderer/RenderPipelineLayers/DefaultRenderPipelineLayer/DefaultRenderPipelineLayer.h"

#include "GameLayer/GameLayer.h"
#include <imgui_impl_sdl2.h>

namespace mrs {
	class EditorLayer : public Layer
	{
	public:
		virtual void OnAttach() override
		{
			LoadEditorResources();
			LoadScene();

			_name = "EditorLayer";
		}

		virtual void OnEvent(Event &event)
		{
			if (Input::IsKeyPressed(SDLK_ESCAPE))
			{
				Pause();
			}

			if (!_playing)
			{
				ImGui_ImplSDL2_ProcessEvent(&event._event);
			}
		};

		virtual void OnDetatch() override
		{
		}

		virtual void OnEnable()
		{
		};

		virtual void OnUpdate(float dt) override
		{
			// Pause on start start
			static bool start_paused = [&]() {
				Pause();
				return true;
				}();

				_dt = dt;
		}

		// Focuses editor on entity
		void FocusEntity(Entity entity)
		{
			EditorManager *editor_script = (EditorManager *)(void *)(_editor_manager.GetComponent<Script>().script);
			_selected_entity = entity;
			if (editor_script != nullptr)
			{
				editor_script->_camera_controller->_focused = entity;
			}
		}

		virtual void OnImGuiRender() override
		{
			// Scene
			Scene *scene = Application::GetInstance().GetScene();
			auto view = scene->Registry()->view<Tag, Transform>();

			MainMenu::Draw(scene);

			// [Ui] Hierarchy Panel
			ImGui::Begin("Entity Hierarchy");
			int ctr = 0;
			for (auto entity : view)
			{
				ImGui::PushID(ctr++);

				Entity e = { entity, scene };
				Tag &tag = e.GetComponent<Tag>();

				if (ImGui::Selectable(tag.tag.c_str(), e == _selected_entity))
				{
					if (ImGui::IsMouseClicked(1))
					{
						ImGui::OpenPopup("Object Options");
					}

					if (ImGui::BeginPopup("Object Options"))
					{
						ImGui::Button("Delete Object");
						ImGui::EndPopup();
					}

					FocusEntity(e);
				}

				ImGui::PopID();
			}
			ImGui::End();

			// [UI] Inspector Panel
			InspectorPanel::Draw(_selected_entity);

			ImGui::Begin("Scene Bar");
			std::string state = _playing ? "Pause" : "Play";
			if (ImGui::Button(state.c_str()))
			{
				if (_playing)
				{
					Pause();
				}
				else
				{
					Play();
				}
			}
			ImGui::End();
		}
	private:
		void Play()
		{
			Application::GetInstance().EnableLayer("Physics2DLayer");
			Application::GetInstance().EnableLayer("NativeScriptingLayer");

			// Disable editor controls and camera 
			EditorManager *em_script = dynamic_cast<EditorManager *>(_editor_manager.GetComponent<Script>().script);
			em_script->_camera.GetComponent<Camera>().SetActive(false);

			_playing = true;
		}

		void Pause()
		{
			// Disable play time layers
			Application::GetInstance().DisableLayer("Physics2DLayer");
			Application::GetInstance().DisableLayer("NativeScriptingLayer");

			// Enable editor controls and camera 
			EditorManager *em_script = dynamic_cast<EditorManager *>(_editor_manager.GetComponent<Script>().script);
			em_script->_camera.GetComponent<Camera>().SetActive(true);

			_playing = false;
		}

		void LoadEditorResources()
		{
			Mesh::LoadFromAsset("Assets/Models/sphere.boop_obj", "sphere");
			Texture::LoadFromAsset("Assets/Models/white.boop_png", "default_texture");
			Material::Create("default_material");

			Mesh::LoadFromAsset("Assets/Models/container.boop_obj", "container");
			Texture::LoadFromAsset("Assets/Models/textures_container/Container_DiffuseMap.boop_jpg", "container");
			Material::Create("container", "container");

			Texture::LoadFromAsset("Assets/Textures/green.boop_png", "green");
			Material::Create("green_material", "green");

			Texture::LoadFromAsset("Assets/Textures/smoke_01.boop_png", "smoke_01");
			Material::Create("smoke_01_material", "smoke_01");

			Mesh::LoadFromAsset("Assets/Models/cube.boop_obj", "cube");
			Mesh::LoadFromAsset("Assets/Models/cone.boop_obj", "cone");
			Mesh::LoadFromAsset("Assets/Models/monkey_smooth.boop_obj", "monkey");
			Mesh::LoadFromAsset("Assets/Models/quad.boop_obj", "quad");

			// Post processing demo
			Texture::LoadFromAsset("Assets/Textures/ChicagoTraffic.boop_jpg", "chicago_traffic");

			auto screen_quad = Mesh::Create("screen_quad");

			screen_quad->_vertices.push_back({ { -1.0f, -1.0f, 0.0f }, {}, {}, {} });
			screen_quad->_vertices.push_back({ { -1.0f, 1.0f, 0.0f }, {}, {}, {} });
			screen_quad->_vertices.push_back({ { 1.0f, -1.0f, 0.0f }, {}, {}, {} });
			screen_quad->_vertices.push_back({ { 1.0f,  1.0f, 0.0f }, {}, {}, {} });

			screen_quad->_vertex_count = 4;

			screen_quad->_indices = { 0,2,1,1,2,3 };
			screen_quad->_index_count = 6;
		}

		void LoadScene()
		{
			// Register Scripts
			Script::Register<CameraController>();
			Script::Register<EditorManager>();

			// Create editor manager
			Scene *scene = Application::GetInstance().GetScene();
			_editor_manager = scene->Instantiate("Editor Manager");
			_editor_manager.AddComponent<Script>().Bind<EditorManager>();
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

		Entity _selected_entity = {};
		Entity _editor_manager = {};

		bool _playing = false;
		bool _initialized = false;
	};

	class Rover : public Application
	{
	public:
		Rover() : Application("Rover", 1600, 900)
		{
			// Default layers
			PushLayer(new InputLayer());
			PushLayer(new DefaultRenderPipelineLayer());
			PushLayer(new Physics2DLayer());
			PushLayer(new NativeScriptingLayer());

			// Client application layers
			PushLayer(new EditorLayer());
			PushLayer(new GameLayer());
		}

		~Rover() {};
	};
}

mrs::Application *mrs::CreateApplication()
{
	return MRS_NEW Rover();
}

