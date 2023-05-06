#ifndef VULKANMESH_H
#define VULKANMESH_H

#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "VulkanStructures.h"

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

	private:
		std::vector<Vertex> _vertices;
		friend class Renderer;
		AllocatedBuffer _buffer;
	};
}

#endif