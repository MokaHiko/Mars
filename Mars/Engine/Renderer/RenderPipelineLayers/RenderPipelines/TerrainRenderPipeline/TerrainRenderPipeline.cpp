#include "TerrainRenderPipeline.h"
#include "TerrainComponent.h"
#include <ECS/Components/Components.h>

void mrs::TerrainRenderPipeline::Init()
{
	_quad_mesh = Mesh::Get("quad");

	// Load height maps
	auto view = _scene->Registry()->view<Transform, TerrainRenderer>();
	for (entt::entity entity: view)
	{
		Entity e = { entity , _scene};
		TerrainRenderer& terrain = e.GetComponent<TerrainRenderer>();

		vkutil::DescriptorBuilder _descriptor_builder;
		//_descriptor_builder.Begin(_renderer->_descriptor_layout_cache.get(), _renderer->_descriptor_allocator.get())
		//	.BindBuffer(0, terrain.terrain_buffer.buffer, );
	}

	// Create pipeline 
}

void mrs::TerrainRenderPipeline::Begin(VkCommandBuffer cmd, uint32_t current_frame)
{
}

