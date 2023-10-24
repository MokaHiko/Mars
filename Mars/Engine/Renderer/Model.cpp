#include "Model.h"

#include <boop.h>
#include <common/model.h>

#include "Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanTexture.h"
#include "Renderer/Vulkan/VulkanMaterial.h"
#include "Renderer/Vulkan/VulkanAssetManager.h"

Ref<mrs::Model> mrs::Model::LoadFromAsset(const std::string& path, bool flip_uvs, const std::string& alias) 
{
	if(Model::Get(path) != nullptr)
	{
		return Model::Get(path);
	}

	Ref<Model> model = CreateRef<Model>();

	if(!alias.empty())
	{
		ResourceManager::Get()._models[alias] = model;
		model->_name = alias;
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

	// Load model materials & textures
	Ref<mrs::EffectTemplate> default_lit = VulkanAssetManager::Instance().FindEffectTemplate("default_lit");
	for (auto i = 0; i < info.material_count; i++)
	{
		const auto& material_info = info.materials[i];
		auto mesh_material = Material::Create(default_lit, "default_texture", material_info.name);
		auto& material_data = mesh_material->Data();

		// Colors
		material_data.diffuse_color = glm::vec4(material_info.diffuse_color.x, material_info.diffuse_color.y, material_info.diffuse_color.z, 1.0f);
		//material_data.specular = material_info.specular_color;

		if (!material_info.diffuse_texture_path.empty())
		{
			const std::string path = "Assets/Models/" + material_info.diffuse_texture_path;
			auto diffuse = Texture::LoadFromAsset(path);
			if(diffuse)
			{
				mesh_material->SetTexture(MaterialTextureType::DiffuseTexture, diffuse);
			}
			else
			{
				MRS_ERROR("No texture at %s found", path.data());
			}
		}
		else
		{ 
			mesh_material->SetTexture(MaterialTextureType::DiffuseTexture, Texture::Get("default")); 
		}

		if (!material_info.specular_texture_path.empty())
		{
			const std::string path = "Assets/Models/" + material_info.specular_texture_path;
			auto specular = Texture::LoadFromAsset(path);
			if (specular)
			{
				mesh_material->SetTexture(MaterialTextureType::SpecularTexture, specular);
			}
			else
			{
				MRS_ERROR("No texture at %s found", path.data());
			}
		}

		model->_materials.push_back(mesh_material);
	}

	// Load each mesh in model
	uint32_t vertex_offset = 0;
	uint32_t index_offset = 0;

	for(auto i = 0 ; i < info.mesh_count; i++)
	{
		auto mesh = Mesh::Create(info.mesh_infos[i].mesh_name + std::to_string(i));

		int index = info.mesh_infos[i].material_index;

		mesh->_vertices.resize(info.mesh_infos[i].vertex_buffer_size / sizeof(Vertex));
		mesh->_indices.resize(info.mesh_infos[i].index_buffer_size / sizeof(uint32_t));

		mesh->_vertex_count = mesh->_vertices.size();
		mesh->_index_count = mesh->_indices.size();

		memcpy(mesh->_vertices.data(), file.raw_data.data() + vertex_offset, info.mesh_infos[i].vertex_buffer_size);

		if (flip_uvs)
		{
			for (auto& vertex : mesh->_vertices)
			{
				vertex.uv.y *= -1;
			}
		}

		memcpy(mesh->_indices.data(), file.raw_data.data() + info.vertex_buffer_size + index_offset, info.mesh_infos[i].index_buffer_size);

		vertex_offset += info.mesh_infos[i].vertex_buffer_size;
		index_offset += info.mesh_infos[i].index_buffer_size;

		ModelMesh model_mesh = { mesh, index };
		model->_meshes.push_back(model_mesh);
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

const std::string& mrs::Model::Name() const
{
	return _name;
}

std::string& mrs::Model::Name()
{
	return _name;
}

