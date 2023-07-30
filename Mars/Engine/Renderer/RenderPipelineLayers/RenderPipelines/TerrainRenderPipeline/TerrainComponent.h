#ifndef TERRAINCOMPONENT_H
#define TERRAINCOMPONENT_H

#pragma once

#include <glm/glm.hpp>
#include "Renderer/Vulkan/VulkanTexture.h"

namespace mrs
{
	class TerrainRenderer
	{
	public:
		glm::vec2 grid_size;
		Texture height_map;
	private:
		friend class TerrainRenderPipeline;
		AllocatedBuffer terrain_buffer;
		VkDescriptorSet terrain_descriptor_set;
	};
}

#endif