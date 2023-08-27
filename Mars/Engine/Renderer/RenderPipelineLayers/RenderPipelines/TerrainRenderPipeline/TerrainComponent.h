#ifndef TERRAINCOMPONENT_H
#define TERRAINCOMPONENT_H

#pragma once

#include <glm/glm.hpp>
#include "Renderer/Vulkan/VulkanTexture.h"
#include "Renderer/Vulkan/VulkanMesh.h"

namespace mrs
{
	enum class TerrainType : uint8_t 
	{
		Cpu,
		Tesselation,
	};

	class TerrainRenderer
	{
	public:
		glm::vec2 grid_size;
		Ref<Texture> height_map;

		TerrainType type = TerrainType::Cpu;

		// TODO: Move to CPUTerrainRenderer class
		Ref<Mesh> _terrain_mesh;
	private:
		friend class TerrainRenderPipeline;
		AllocatedBuffer terrain_buffer;
		VkDescriptorSet terrain_descriptor_set;
	};
}

#endif