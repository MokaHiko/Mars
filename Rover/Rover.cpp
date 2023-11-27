#include "Rover.h"

#include <iostream>

#include <Core/Input.h>
#include <Core/Log.h>
#include <Core/Memory.h>
#include <Renderer/Model.h>

#include <Physics/Physics2DLayer.h>
#include <Scripting/ProcessLayer.h>

#include "Scripts/Editor/EditorManager.h"
#include "UIHelpers.h"

#include "Panels/Menus/MainMenu.h"

#include "Core/InputLayer.h"
#include "Scripting/NativeScriptingLayer.h"
#include "Renderer/RenderPipelineLayers/DefaultRenderPipelineLayer/DefaultRenderPipelineLayer.h"
#include "SceneGraph/SceneGraphLayer.h"

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

Ref<mrs::IPanel> mrs::EditorLayer::FindPanel(const std::string& name)
{
	for (Ref<IPanel> panel : _panels)
	{
		if (panel->Name() == name)
		{
			return panel;
		}
	}

	return nullptr;
}

void mrs::EditorLayer::FocusEntity(Entity entity)
{
	_selected_entity = entity;

	if (!_editor_camera.HasComponent<Script>())
	{
		return;
	}

	CameraController* camera_controller = (CameraController*)(void*)(_editor_camera.GetComponent<Script>().script);
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
	// ~ Escape Velocity
	{
		// Ships
		Model::LoadFromAsset("Assets/Models/ships/Zenith.bp", true, "zenith");
		Model::LoadFromAsset("Assets/Models/ships/Striker.bp", true, "striker");

		// Default materials
		Ref<Texture> coin_texture = Texture::LoadFromAsset("Assets/Textures/coin.bp", "coin");
		Material::Create(VulkanAssetManager::Instance().FindEffectTemplate("default_lit"), coin_texture , "coin");

		// Create Planet Material
		std::vector<mrs::ShaderEffect*> cb_effects;
		cb_effects.push_back(_render_pipeline_layer->FindPipeline("CBRenderPipeline")->Effect().get());
		Ref<mrs::EffectTemplate> cb_effect = VulkanAssetManager::Instance().CreateEffectTemplate(cb_effects, "celestial_body_effect");

		Ref<Texture> smoke_texture = Texture::LoadFromAsset("Assets/Textures/smoke.bp", "smoke");
		Material::Create(VulkanAssetManager::Instance().FindEffectTemplate("default_particle"),  smoke_texture, "smoke");

		Material::Create(cb_effect, Texture::Get("default"), "celestial_body");
	}

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
	scene->Destroy(_editor_camera);

	// Enable runtime layers
	application.EnableLayer("Physics2DLayer");
	_native_scripting_layer->EnableScripts();

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

	// Custom render pipeline
	PushLayer(MRS_NEW EVRenderPipelineLayer());

	PushLayer(MRS_NEW Physics2DLayer());
	PushLayer(MRS_NEW NativeScriptingLayer());
	PushLayer(MRS_NEW ProcessLayer());
	PushLayer(MRS_NEW SceneGraphLayer());

	// Client layers
	PushLayer(MRS_NEW EditorLayer());
	PushLayer(MRS_NEW GameLayer());
}

mrs::Rover::~Rover() {}

