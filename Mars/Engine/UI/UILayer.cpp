#include "UILayer.h"
#include "Core/Application.h"

#include "UI/UI.h"
#include "Renderer/Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanAssetManager.h"

mrs::UILayer::UILayer() 
	:Layer("UILayer") {}

mrs::UILayer::~UILayer() {}

void mrs::UILayer::OnEnable() 
{
	_scene = Application::Instance().GetScene();
}

void mrs::UILayer::OnDisable() 
{
	_scene = nullptr;
}

void mrs::UILayer::OnUpdate(float dt) 
{
	// Generated vertices/meshes for text
	auto view = _scene->Registry()->view<SpriteRenderer, Text>();

	for(auto& entity : view)
	{
		Entity e(entity, _scene);
		auto& sprite_renderer = e.GetComponent<SpriteRenderer>();
		auto& text_component = e.GetComponent<Text>();

		if(!text_component.dirty)
		{
			continue;
		}

		std::string mesh_name = "Text_Quads_" + std::to_string(text_component.text.size());
		sprite_renderer.mesh = Mesh::Get(mesh_name);

		if(sprite_renderer.mesh == nullptr)
		{
			sprite_renderer.mesh = Mesh::Create(mesh_name);
		}

		int ctr = 0;
		Rectangle atlas_rect = {};
		atlas_rect.height = sprite_renderer.sprite->Atlas()->_height;
		atlas_rect.width = sprite_renderer.sprite->Atlas()->_width;
		
		for(const char c : text_component.text)
		{
			const Rectangle& rect = text_component.font->GetUVS(c);

			// Create Vertices for each character
			sprite_renderer.mesh->_vertices.push_back({Vector3{-1.00, -1.00,  0.00} + Vector3(static_cast<float>(ctr) * 2.0f, 0, 0), {0.00, 0.00, 0.00},  {0.00,  0.00,  1.00},  Vector2{0.00,  1.00} * Vector2(rect.width /atlas_rect.width, rect.height / atlas_rect.height) + Vector2(rect.x / atlas_rect.width, rect.y / atlas_rect.height)});
			sprite_renderer.mesh->_vertices.push_back({Vector3{ 1.00, -1.00,  0.00} + Vector3(static_cast<float>(ctr) * 2.0f, 0, 0), {0.00, 0.00, 0.00},  {0.00,  0.00,  1.00},  Vector2{1.00,  1.00} * Vector2(rect.width /atlas_rect.width, rect.height / atlas_rect.height) + Vector2(rect.x / atlas_rect.width, rect.y / atlas_rect.height)});
			sprite_renderer.mesh->_vertices.push_back({Vector3{-1.00,  1.00,  0.00} + Vector3(static_cast<float>(ctr) * 2.0f, 0, 0), {0.00, 0.00, 0.00},  {0.00,  0.00,  1.00},  Vector2{0.00,  0.00} * Vector2(rect.width /atlas_rect.width, rect.height / atlas_rect.height) + Vector2(rect.x / atlas_rect.width, rect.y / atlas_rect.height)});
			sprite_renderer.mesh->_vertices.push_back({Vector3{ 1.00, -1.00,  0.00} + Vector3(static_cast<float>(ctr) * 2.0f, 0, 0), {0.00, 0.00, 0.00},  {0.00,  0.00,  1.00},  Vector2{1.00,  1.00} * Vector2(rect.width /atlas_rect.width, rect.height / atlas_rect.height) + Vector2(rect.x / atlas_rect.width, rect.y / atlas_rect.height)});
			sprite_renderer.mesh->_vertices.push_back({Vector3{ 1.00,  1.00,  0.00} + Vector3(static_cast<float>(ctr) * 2.0f, 0, 0), {0.00, 0.00, 0.00},  {0.00,  0.00,  1.00},  Vector2{1.00,  0.00} * Vector2(rect.width /atlas_rect.width, rect.height / atlas_rect.height) + Vector2(rect.x / atlas_rect.width, rect.y / atlas_rect.height)});
			sprite_renderer.mesh->_vertices.push_back({Vector3{-1.00,  1.00,  0.00} + Vector3(static_cast<float>(ctr) * 2.0f, 0, 0), {0.00, 0.00, 0.00},  {0.00,  0.00,  1.00},  Vector2{0.00,  0.00} * Vector2(rect.width /atlas_rect.width, rect.height / atlas_rect.height) + Vector2(rect.x / atlas_rect.width, rect.y / atlas_rect.height)});

			sprite_renderer.mesh->_vertex_count += 6;
			ctr++;
		}

		for(int i = 0; i < sprite_renderer.mesh->_vertex_count; i++)
		{
			sprite_renderer.mesh->_indices.push_back(i);
		}
		sprite_renderer.mesh->_index_count = sprite_renderer.mesh->_vertex_count;


		VulkanAssetManager::Instance().UploadMesh(sprite_renderer.mesh);
		text_component.dirty = false;
	}
}
