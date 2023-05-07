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

		static VertexInputDescription& GetDescription();
	};

	class Mesh
	{
	public:
		Mesh() {};
		~Mesh() {};

		static std::shared_ptr<Mesh> Create(const std::string& path)
		{
			ResourceManager::Get()._meshes[path] = std::make_shared<Mesh>();
			return ResourceManager::Get()._meshes[path];
		}

		static std::shared_ptr<Mesh> Get(const std::string& path)
		{
			auto it = ResourceManager::Get()._meshes.find(path);

			if (it != ResourceManager::Get()._meshes.end()) {
				return it->second;
			}

			return nullptr;
		}

		std::vector<Vertex> _vertices;
	private:
		friend class Renderer;
		AllocatedBuffer _buffer;
	};
}

#endif