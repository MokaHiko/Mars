#include <iostream>

#include <Mars.h>
#include <Core/Input.h>
#include <Core/Log.h>

#include <Physics/Physics2DLayer.h>

#include "Scripts/Editor/EditorManager.h"
#include "UIHelpers.h"

#include "Panels/Menus/MainMenu.h"
#include "Panels/Menus/Viewport.h"
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
		enum class EditorState : uint8_t
		{
			Uknown,
			Playing, 
			Paused,
			Stopped,
		};

		virtual void OnAttach() override
		{
			LoadEditorResources();
			_name = "EditorLayer";
		}

		virtual void OnEvent(Event &event)
		{
			if (Input::IsKeyPressed(SDLK_ESCAPE))
			{
				Stop();
			}

			ImGui_ImplSDL2_ProcessEvent(&event._event);
		};

		virtual void OnDetatch() override
		{
		}

		virtual void OnEnable()
		{
			_native_scripting_layer = (NativeScriptingLayer*)(void*)Application::GetInstance().FindLayer("NativeScriptingLayer");
			_render_pipeline_layer = (IRenderPipelineLayer*)(void*)Application::GetInstance().FindLayer("IRenderPipelineLayer");

			Viewport::Init(_render_pipeline_layer);
		};

		virtual void OnUpdate(float dt) override
		{
			static bool first_run = [&]()
			{
				Stop();
				return true;
			}();
		}

		// Focuses editor on entity
		void FocusEntity(Entity entity)
		{
			CameraController *camera_controller = (CameraController*)(void *)(_editor_camera.GetComponent<Script>().script);
			_selected_entity = entity;

			if (camera_controller != nullptr)
			{
				camera_controller->_focused = entity;
			}
		}

		virtual void OnImGuiRender() override
		{
			Scene *scene = Application::GetInstance().GetScene();
			auto view = scene->Registry()->view<Tag, Transform>();

			MainMenu::Draw(scene);
			Viewport::Draw();

			// [Ui] Hierarchy Panel
			ImGui::Begin("Entity Hierarchy");
			int ctr = 0;

			// Select entity
			for (auto entity : view)
			{
				ImGui::PushID(ctr++);

				Entity e = { entity, scene };
				Tag &tag = e.GetComponent<Tag>();

				if (ImGui::Selectable(tag.tag.c_str(), e == _selected_entity))
				{
					if (ImGui::IsMouseClicked(1))
					{
						MRS_INFO("Object options!");
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

			// Creating entity
			if(ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow))
			{
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					ImGui::OpenPopup("Create Options");
				}
			}

			if (ImGui::BeginPopup("Create Options"))
			{
				if(ImGui::Button("New Entity"))
				{
					scene->Instantiate("New Entity");
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			ImGui::End();

			// [UI] Inspector Panel
			InspectorPanel::Draw(_selected_entity);

			ImGui::Begin("Scene Bar");
			std::string state = _state == EditorState::Playing ? "Pause" : "Play";
			if (ImGui::Button(state.c_str()))
			{
				if (_state == EditorState::Playing)
				{
					Pause();
				}
				else
				{
					Play();
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Stop"))
			{
				Stop();
			}
			ImGui::End();
		}
	private:
		void Play()
		{
			auto& application = Application::GetInstance();

			// Disable serialization
			application.GetScene()->Serialization(false);

			// Destroy editor scene entities
			_editor_camera.GetComponent<Camera>().SetActive(false);
			Scene *scene = application.GetScene();
			auto& view = scene->Registry()->view<Transform, Serializer>();
			for(auto entity : view)
			{
				Entity e(entity, scene);
				auto& serializer = e.GetComponent<Serializer>();

				if(!serializer.serialize)
				{
					scene->Destroy(e);
				}
			}

			// Enable play time layers
			application.EnableLayer("Physics2DLayer");
			_native_scripting_layer->EnableScripts(_editor_camera);

			_state = EditorState::Playing;
		}

		void Pause()
		{
			_state = EditorState::Paused;
		}

		void Stop()
		{
			if (_state == EditorState::Stopped)
			{
				return;
			}

			auto& application = Application::GetInstance();

			// Destroy runtime created entites
			Scene *scene = application.GetScene();
			auto& view = scene->Registry()->view<Transform, Serializer>();

			for(auto entity : view)
			{
				Entity e(entity, scene);
				auto& serializer = e.GetComponent<Serializer>();

				if(!serializer.serialize)
				{
					scene->Destroy(e);
				}
			}

			// Disable play time layers
			application.DisableLayer("Physics2DLayer");
			_native_scripting_layer->DisableScripts(_editor_camera);

			// Load editor scene 
			LoadEditorScene();

			// Enable serialization
			application.GetScene()->Serialization(true);

			_state = EditorState::Stopped;
		}

		void LoadEditorResources()
		{
			Mesh::LoadFromAsset("Assets/Models/sphere.boop_obj", "sphere");
			Texture::LoadFromAsset("Assets/Models/white.boop_png", "default_texture");
			Material::Create("default_material");

			Mesh::LoadFromAsset("Assets/Models/container.boop_obj", "container");
			Texture::LoadFromAsset("Assets/Models/textures_container/Container_DiffuseMap.boop_jpg", "container");
			Material::Create("container_material", "container");

			Texture::LoadFromAsset("Assets/Textures/green.boop_png", "green");
			Material::Create("green_material", "green");

			Texture::LoadFromAsset("Assets/Textures/smoke_01.boop_png", "smoke_01");
			Material::Create("smoke_01_material", "smoke_01");

			Mesh::LoadFromAsset("Assets/Models/cube.boop_obj", "cube");
			Mesh::LoadFromAsset("Assets/Models/cone.boop_obj", "cone");
			Mesh::LoadFromAsset("Assets/Models/monkey_smooth.boop_obj", "monkey");
			Mesh::LoadFromAsset("Assets/Models/quad.boop_obj", "quad");

			// Manually built meshes
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

		void LoadEditorScene()
		{
			MRS_INFO("Initializing Editor");

			// Register Scripts
			Script::Register<CameraController>();

			bool serialize = false;

			// Instantiate Camera
			auto& app = Application::GetInstance();
			auto window = app.GetWindow();
			_editor_camera = app.GetScene()->Instantiate("Editor Camera", {}, &serialize);

			auto& camera_component =_editor_camera.AddComponent<Camera>(CameraType::Perspective, window->GetWidth(), window->GetHeight());

    		_editor_camera.AddComponent<Script>().Bind<CameraController>();
			_editor_camera.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, 50.0f);

			_render_pipeline_layer->SetCamera(&camera_component);
		}

	private:
		float _mouse_sensitivity = 0.5f;
		bool _constrain_pitch = true;
		float _movement_speed = 50.0f;

		Entity _selected_entity = {};
		Entity _editor_camera = {};

		EditorState _state;

		NativeScriptingLayer* _native_scripting_layer = nullptr;
		IRenderPipelineLayer* _render_pipeline_layer = nullptr;
	};

	class Rover : public Application
	{
	public:
		Rover() : Application("Rover", 1600, 900)
		{
			// Default layers
			PushLayer(MRS_NEW InputLayer());
			PushLayer(MRS_NEW DefaultRenderPipelineLayer());
			PushLayer(MRS_NEW Physics2DLayer());
			PushLayer(MRS_NEW NativeScriptingLayer());

			// Client application layers
			PushLayer(MRS_NEW EditorLayer());
			PushLayer(MRS_NEW GameLayer());
		}

		~Rover() {};
	};
}

mrs::Application *mrs::CreateApplication()
{
	return MRS_NEW Rover();
}

