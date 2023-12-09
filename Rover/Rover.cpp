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
#include "UI/UILayer.h"

#include "GameLayer/GameLayer.h"
#include <imgui_impl_sdl2.h>

#include "Panels/IPanel.h"
#include "Panels/PerformancePanel.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/Viewport.h"

#include "UI/UI.h"

// TODO: Remove
#include "Renderer/RenderPipelineLayers/RenderPipelines/TrailRenderPipeline/Trails.h"

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
	if(!entity.IsAlive())
	{
		MRS_INFO("Cannot focus on invalid entity!");
		return;
	}

	_selected_entity = entity;

	if (!_editor_camera.HasComponent<Script>())
	{
		return;
	}

	EditorCameraController* camera_controller = (EditorCameraController*)(void*)(_editor_camera.GetComponent<Script>().script);
	if (camera_controller != nullptr)
	{
		camera_controller->_focused = entity;
	}
}

void mrs::EditorLayer::ToggleInput(bool enable) 
{
	auto& application = Application::Instance();

	if(enable)
	{
		application.EnableLayer("InputLayer");
	}
	else
	{
		application.DisableLayer("InputLayer");
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
	_panels.push_back(CreateRef<MainMenu>(*this, "MainMenu", scene));
	_panels.push_back(CreateRef<HierarchyPanel>(*this, "Hierarchy", scene));
	_panels.push_back(CreateRef<Viewport>(*this, "ViewPort", _render_pipeline_layer));
	_panels.push_back(CreateRef<PerformancePanel>(*this, "Performance Panel", _render_pipeline_layer));

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
	application.DisableLayer("InputLayer");
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

		Ref<Texture> stars_texture = Texture::LoadFromAsset("Assets/Textures/stars.bp", "stars");
		stars_texture->SetSamplerType(mrs::Texture::SamplerType::Nearest);

		// Skybox texture should not receive specular lighting
		Material::Create(VulkanAssetManager::Instance().FindEffectTemplate("default_lit"), stars_texture , "stars")->Data().specular = 0.0f;

		// Sprites
		mrs::Texture::LoadFromAsset("Assets/Textures/Sprites/target_sheet.bp", "target_sprite_sheet");
		Sprite::Create(mrs::Texture::Get("target_sprite_sheet"), "Assets/Textures/Sprites/target_sheet.yaml", "target_sprite_sheet");
		Sprite::Create(mrs::Texture::Get("KenPixel"));
		
		mrs::Texture::LoadFromAsset("Assets/Textures/target_lock.bp", "target_lock");
		Sprite::Create(mrs::Texture::Get("target_lock"));

		// Create Planet Material
		std::vector<mrs::ShaderEffect*> cb_effects;
		cb_effects.push_back(_render_pipeline_layer->FindPipeline("CBRenderPipeline")->Effect().get());
		Ref<mrs::EffectTemplate> cb_effect = VulkanAssetManager::Instance().CreateEffectTemplate(cb_effects, "celestial_body_effect");

		Ref<Texture> smoke_texture = Texture::LoadFromAsset("Assets/Textures/smoke.bp", "smoke");
		Material::Create(VulkanAssetManager::Instance().FindEffectTemplate("default_particle"),  smoke_texture, "smoke");

		Ref<Texture> muzzle_texture = Texture::LoadFromAsset("Assets/Textures/muzzle_02.bp", "muzzle_02");
		Material::Create(VulkanAssetManager::Instance().FindEffectTemplate("default_particle"), muzzle_texture, "muzzle_02");

		Ref<Texture> fire_texture = Texture::LoadFromAsset("Assets/Textures/fire_01.bp", "fire_01");
		Material::Create(VulkanAssetManager::Instance().FindEffectTemplate("default_particle"), fire_texture, "fire_01");

		Ref<Texture> trace_texture = Texture::LoadFromAsset("Assets/Textures/trace_01.bp", "trace_01");
		Material::Create(VulkanAssetManager::Instance().FindEffectTemplate("default_particle"), trace_texture, "trace_01");

		Material::Create(cb_effect, Texture::Get("default"), "celestial_body");
	}

	// Upload resources to runtime
	_render_pipeline_layer->UploadResources();
}

void mrs::EditorLayer::LoadEditorScene()
{
	MRS_INFO("Initializing Editor");

	// Register Scripts
	Script::Register<EditorCameraController>();

	bool serialize = false;

	// Instantiate Camera
	auto& app = Application::Instance();
	auto window = app.GetWindow();
	_editor_camera = app.GetScene()->Instantiate("Editor Camera", {}, &serialize);

	auto& camera_component = _editor_camera.AddComponent<Camera>(CameraType::Perspective, window->GetWidth(), window->GetHeight());

	_editor_camera.AddScript<EditorCameraController>();
	_editor_camera.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, 50.0f);

	_render_pipeline_layer->SetCamera(&camera_component);

	// Demo Text
	auto hello_world = app.GetScene()->Instantiate("Test Text", {}, &serialize);
	hello_world.AddComponent<SpriteRenderer>().sprite = mrs::Sprite::Get("KenPixel");
	hello_world.AddComponent<Renderable>().material = mrs::Material::Get("default_ui");
	hello_world.AddComponent<Text>().font = Font::LoadFromYaml(mrs::Texture::Get("KenPixel"), "Assets/Fonts/KenneyPixel.yaml");;
	hello_world.AddComponent<Text>().text = "eat shit kurt!";

	// Demo Trail
	auto trails = app.GetScene()->Instantiate("Test Trail", {}, &serialize);
	trails.AddComponent<TrailRenderer>();
	trails.AddComponent<Renderable>().material = mrs::Material::Get("default_line");
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
	application.EnableLayer("InputLayer");
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

	PushLayer(MRS_NEW Physics2DLayer());
	PushLayer(MRS_NEW NativeScriptingLayer());
	PushLayer(MRS_NEW ProcessLayer());
	PushLayer(MRS_NEW SceneGraphLayer());

	PushLayer(MRS_NEW UILayer());

	// Custom render pipeline
	PushLayer(MRS_NEW EVRenderPipelineLayer());

	// Client layers
	PushLayer(MRS_NEW EditorLayer());
	PushLayer(MRS_NEW GameLayer());
}

mrs::Rover::~Rover() {}

