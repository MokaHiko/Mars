#include "Model.h"

#include <boop.h>
#include <common/model.h>

#include "Vulkan/VulkanMesh.h"

Ref<mrs::Model> mrs::Model::LoadFromAsset(const std::string& path, const std::string& alias)
{
	if(Model::Get(path) != nullptr)
	{
		return Model::Get(path);
	}

	Ref<Model> model = CreateRef<Model>();

	if(!alias.empty())
	{
		ResourceManager::Get()._models[alias] = model;
	}
	else
	{
		ResourceManager::Get()._models[alias] = model;
	}

	// Load asset file
    boop::AssetFile file; 
    boop::load(path.c_str(), file);

	boop::ModelInfo info = boop::read_model_info(&file);

	char* vertex_buffer = (char*)malloc(info.vertex_buffer_size);
	char* index_buffer = (char*)malloc(info.index_buffer_size);

	boop::unpack_model(&info, file.raw_data.data(), file.raw_data.size(), vertex_buffer, index_buffer);

	// Load each mesh in model
	uint32_t vertex_offset = 0;
	uint32_t index_offset = 0;
	for(auto i = 0 ; i < info.mesh_count; i++)
	{
		auto mesh = Mesh::Create(info.mesh_infos[i].mesh_name + std::to_string(i));

		mesh->_vertices.resize(info.mesh_infos[i].vertex_buffer_size / sizeof(Vertex));
		mesh->_indices.resize(info.mesh_infos[i].index_buffer_size / sizeof(uint32_t));

		mesh->_vertex_count = mesh->_vertices.size();
		mesh->_index_count = mesh->_indices.size();

		memcpy(mesh->_vertices.data(), file.raw_data.data() + vertex_offset, info.mesh_infos[i].vertex_buffer_size);
		memcpy(mesh->_indices.data(), file.raw_data.data() + info.vertex_buffer_size + index_offset, info.mesh_infos[i].index_buffer_size);
		model->_meshes.push_back(mesh);

		vertex_offset += info.mesh_infos[i].vertex_buffer_size;
		index_offset += info.mesh_infos[i].index_buffer_size;
	}

	free(vertex_buffer);
	free(index_buffer);

    return model;
}


Ref<mrs::Model> mrs::Model::Create(const std::string& alias)
{
	ResourceManager::Get()._models[alias] = std::make_shared<Model>();

	return ResourceManager::Get()._models[alias];
}

Ref<mrs::Model> mrs::Model::Get(const std::string& alias)
{
	auto it = ResourceManager::Get()._models.find(alias);

	if (it != ResourceManager::Get()._models.end()) 
	{
		return it->second;
	}

	return nullptr;
}
