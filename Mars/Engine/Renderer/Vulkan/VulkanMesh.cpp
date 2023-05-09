#include "VulkanMesh.h"

#include <boop.h>

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

std::shared_ptr<mrs::Mesh> mrs::Mesh::LoadFromAsset(const std::string& path)
{
	auto mesh = std::make_shared<Mesh>();
	ResourceManager::Get()._meshes[path] = mesh;

	// Load mesh asset file
	boop::AssetFile asset = {};
	boop::load(path.c_str(), asset);

	// Unpack data into mesh
	boop::MeshInfo mesh_info = boop::read_mesh_info(&asset);

	mesh->_vertices.resize(mesh_info.vertex_buffer_size);
	mesh->_indices.resize(mesh_info.index_buffer_size);

	boop::unpack_mesh(&mesh_info, asset.raw_data.data(), asset.raw_data.size(), (char*)mesh->_vertices.data(), (char*)mesh->_indices.data());

	return mesh;
}
