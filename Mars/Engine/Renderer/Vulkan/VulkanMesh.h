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
		glm::vec3 position;
		glm::vec3 color;
		glm::vec3 normal;
		glm::vec2 uv;

		static VertexInputDescription& GetDescription();
	};

	class Mesh
	{
	public:
		Mesh() {};
		~Mesh() {};

		static std::shared_ptr<Mesh> LoadFromAsset(const std::string& path, const std::string& alias);

		static std::shared_ptr<Mesh> Create(const std::string& path)
		{
			ResourceManager::Get()._meshes[path] = std::make_shared<Mesh>();
			return ResourceManager::Get()._meshes[path];
		}

		static std::shared_ptr<Mesh> Get(const std::string& alias)
		{
			auto it = ResourceManager::Get()._meshes.find(alias);

			if (it != ResourceManager::Get()._meshes.end()) {
				return it->second;
			}

			return nullptr;
		}

		uint32_t _index_count = 0;
		uint32_t _vertex_count = 0;

		std::vector<Vertex> _vertices;
		std::vector<uint32_t> _indices;
	private:
		friend class Renderer;
		AllocatedBuffer _buffer = {};
	};
}

#endif