#include "Rover.h"

#include <iostream>

#include <Core/Input.h>
#include <Core/Log.h>
#include <Core/Memory.h>
#include <Renderer/Model.h>

#include <Physics/Physics2DLayer.h>

#include "Scripts/Editor/EditorManager.h"
#include "UIHelpers.h"

#include "Panels/Menus/MainMenu.h"

#include "Core/InputLayer.h"
#include "Scripting/NativeScriptingLayer.h"
#include "Renderer/RenderPipelineLayers/DefaultRenderPipelineLayer/DefaultRenderPipelineLayer.h"

#include "GameLayer/GameLayer.h"
#include <imgui_impl_sdl2.h>

#include "Panels/IPanel.h"
#include "Panels/PerformancePanel.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/Viewport.h"

mrs::Application* mrs::CreateApplication()
{
	return MRS_NEW Rover();
}

void mrs::EditorLayer::FocusEntity(Entity entity)
{
	 CameraController* camera_controller = (CameraController*)(void*)(_editor_camera.GetComponent<Script>().script);
	 _selected_entity = entity;

	 if (camera_controller != nullptr)
	 {
		camera_controller->_focused = entity;
	 }
}

void mrs::EditorLayer::OnAttach()
{
	_name = "EditorLayer";
}

void mrs::EditorLayer::OnEvent(Event& event)
{
	if (Input::IsKeyPressed(SDLK_ESCAPE))
	{
		Stop();
	}

	ImGui_ImplSDL2_ProcessEvent(&event._event);
}

void mrs::EditorLayer::OnDetatch() {}

void mrs::EditorLayer::OnEnable()
{
	Scene* scene = Application::Instance().GetScene();

	_native_scripting_layer = (NativeScriptingLayer*)(void*)Application::Instance().FindLayer("NativeScriptingLayer");
	_render_pipeline_layer = (IRenderPipelineLayer*)(void*)Application::Instance().FindLayer("IRenderPipelineLayer");

	// Push Panels
	_panels.push_back(CreateRef<MainMenu>(this, "MainMenu", scene));
	_panels.push_back(CreateRef<HierarchyPanel>(this, "Hierarchy", scene));
	_panels.push_back(CreateRef<Viewport>(this, "ViewPort", _render_pipeline_layer));
	_panels.push_back(CreateRef<PerformancePanel>(this, "Performance Panel", _render_pipeline_layer));

	LoadEditorResources();
}

void mrs::EditorLayer::OnUpdate(float dt)
{
	static bool first_run = [&]()
		{
			Stop();
			return true;
		}();
}

void mrs::EditorLayer::OnImGuiRender()
{
	// Loop through each panel
	for (auto it = _panels.begin(); it != _panels.end(); it++)
	{
		(*it)->Draw();
	}
}

void mrs::EditorLayer::Stop()
{
	if (_state == EditorState::Stopped)
	{
		return;
	}

	auto& application = Application::Instance();

	// Destroy runtime created entites
	Scene* scene = application.GetScene();
	auto& view = scene->Registry()->view<Transform, Serializer>();

	for (auto entity : view)
	{
		Entity e(entity, scene);
		auto& serializer = e.GetComponent<Serializer>();

		if (!serializer.serialize)
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

void mrs::EditorLayer::LoadEditorResources()
{
	// Create templates
	std::vector<ShaderEffect*> default_lit_effects;
	default_lit_effects.push_back(_render_pipeline_layer->FindPipeline("MeshRenderPipeline")->Effect().get());
	VulkanAssetManager::Instance().CreateEffectTemplate(default_lit_effects, "default_lit");

	// TODO: Load all assets in asset folder
	//Model::LoadFromAsset("Assets/Models/Room.bp", "room");
	Mesh::LoadFromAsset("Assets/Models/sphere.boop_obj", "sphere");
	Texture::LoadFromAsset("Assets/Models/white.boop_png", "default_texture");
	Material::Create("default_lit", "default_material");

	Mesh::LoadFromAsset("Assets/Models/container.boop_obj", "container");
	Texture::LoadFromAsset("Assets/Models/textures_container/Container_DiffuseMap.boop_jpg", "container");
	Material::Create("default_lit", "container", "container");

	Texture::LoadFromAsset("Assets/Textures/green.boop_png", "green");
	Material::Create("default_lit", "green_material", "green");

	Texture::LoadFromAsset("Assets/Textures/smoke_01.boop_png", "smoke_01");
	Material::Create("default_lit", "smoke_01_material", "smoke_01");

	// Basic mesh shapes
	Mesh::LoadFromAsset("Assets/Models/cube.boop_obj", "cube");
	Mesh::LoadFromAsset("Assets/Models/cone.boop_obj", "cone");
	Mesh::LoadFromAsset("Assets/Models/monkey_smooth.boop_obj", "monkey");
	Mesh::LoadFromAsset("Assets/Models/quad.boop_obj", "quad");

	// Manually built meshes
	auto screen_quad = Mesh::Create("screen_quad");

	screen_quad->_vertices.push_back({ { -1.0f, -1.0f, 0.0f }, {}, {}, {} });
	screen_quad->_vertices.push_back({ { -1.0f, 1.0f, 0.0f }, {}, {}, {} });
	screen_quad->_vertices.push_back({ { 1.0f, -1.0f, 0.0f }, {}, {}, {} });
	screen_quad->_vertices.push_back({ { 1.0f,  1.0f, 0.0f }, {}, {}, {} });

	screen_quad->_vertex_count = 4;

	screen_quad->_indices = { 0,2,1,1,2,3 };
	screen_quad->_index_count = 6;

	// Upload resources to runtime
	_render_pipeline_layer->UploadResources();
}

void mrs::EditorLayer::LoadEditorScene()
{
	MRS_INFO("Initializing Editor");

	// Register Scripts
	Script::Register<CameraController>();

	bool serialize = false;

	// Instantiate Camera
	auto& app = Application::Instance();
	auto window = app.GetWindow();
	_editor_camera = app.GetScene()->Instantiate("Editor Camera", {}, &serialize);

	auto& camera_component = _editor_camera.AddComponent<Camera>(CameraType::Perspective, window->GetWidth(), window->GetHeight());

	_editor_camera.AddComponent<Script>().Bind<CameraController>();
	_editor_camera.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, 50.0f);

	// TODO: Move to Scene Hiechy implementation
	// for (auto mesh : Model::Get("room")->_meshes)
	// {
	// 	Entity e = app.GetScene()->Instantiate(mesh->_mesh_name);
	// 	e.AddComponent<MeshRenderer>(mesh, Material::Get("default_material"));
	// }

	_render_pipeline_layer->SetCamera(&camera_component);
}

void mrs::EditorLayer::Play()
{
	if (_state == EditorState::Playing)
	{
		return;
	}

	auto& application = Application::Instance();

	// Disable serialization
	application.GetScene()->Serialization(false);

	// Destroy editor scene entities
	_editor_camera.GetComponent<Camera>().SetActive(false);
	Scene* scene = application.GetScene();
	auto& view = scene->Registry()->view<Transform, Serializer>();
	for (auto entity : view)
	{
		Entity e(entity, scene);
		auto& serializer = e.GetComponent<Serializer>();

		if (!serializer.serialize)
		{
			scene->Destroy(e);
		}
	}

	// Enable play time layers
	application.EnableLayer("Physics2DLayer");
	_native_scripting_layer->EnableScripts(_editor_camera);

	_state = EditorState::Playing;
}

void mrs::EditorLayer::Pause()
{
	_state = EditorState::Paused;
}

mrs::Rover::Rover()
	: Application("Rover", 1600, 900)
{
	// Default layers
	PushLayer(MRS_NEW InputLayer());
	PushLayer(MRS_NEW DefaultRenderPipelineLayer());
	PushLayer(MRS_NEW Physics2DLayer());
	PushLayer(MRS_NEW NativeScriptingLayer());

	// Client layers
	PushLayer(MRS_NEW EditorLayer());
	PushLayer(MRS_NEW GameLayer());
}

mrs::Rover::~Rover() {}

