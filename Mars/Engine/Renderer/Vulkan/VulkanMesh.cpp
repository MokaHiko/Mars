#include "VulkanMesh.h"

mrs::VertexInputDescription& mrs::Vertex::GetDescription()
{
	static VertexInputDescription description = {};

	if (description.bindings.size() > 0)
		return description;

	VkVertexInputBindingDescription vertex_binding = {};
	vertex_binding.binding = 0;
	vertex_binding.stride = sizeof(Vertex);
	vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	description.bindings.push_back(vertex_binding);

	VkVertexInputAttributeDescription position_atr = {};
	position_atr.binding = 0;
	position_atr.location = 0;
	position_atr.format = VK_FORMAT_R32G32B32_SFLOAT;
	position_atr.offset = offsetof(Vertex, position);

	description.attributes.push_back(position_atr);

	VkVertexInputAttributeDescription color_atr = {};
	color_atr.binding = 0;
	color_atr.location = 1;
	color_atr.format = VK_FORMAT_R32G32B32_SFLOAT;
	color_atr.offset = offsetof(Vertex, color);

	description.attributes.push_back(color_atr);

	return description;
}
