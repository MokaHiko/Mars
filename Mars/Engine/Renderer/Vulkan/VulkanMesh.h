#ifndef VULKANMESH_H
#define VULKANMESH_H

#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "VulkanStructures.h"

#include "Core/ResourceManager.h"

namespace mrs {

	struct VertexInputDescription
	{
		std::vector<VkVertexInputBindingDescription> bindings = {};
		std::vector<VkVertexInputAttributeDescription> attributes = {};
	};

	struct Vertex
	{
		glm::vec3 position{ 0.0f };
		glm::vec3 color{ 1.0f };
		glm::vec3 normal{ 0.0f };
		glm::vec2 uv{ 0.0f };

		static VertexInputDescription& GetDescription();
	};

	class Mesh
	{
	public:
		Mesh() {};
		~Mesh() {};

		static Ref<Mesh> LoadFromAsset(const std::string& path, const std::string& alias);

		static Ref<Mesh> Create(const std::string& alias);

		static Ref<Mesh> Get(const std::string& alias);

		std::string _mesh_name;

		uint32_t _index_count = 0;
		uint32_t _vertex_count = 0;

		std::vector<Vertex> _vertices;
		std::vector<uint32_t> _indices;

		AllocatedBuffer _buffer = {};
		AllocatedBuffer _index_buffer = {};
	};
}

#endif