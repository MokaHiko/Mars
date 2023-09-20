#include "VulkanMaterial.h"

#include "Core/Application.h"
#include "Renderer/RenderPipelineLayers/IRenderPipelineLayer.h"

Ref<mrs::Texture> mrs::Material::MainTexture()
{
	return GetTexture(MaterialTextureType::DiffuseTexture);
}

Ref<mrs::Texture> mrs::Material::GetTexture(MaterialTextureType type)
{
	if(static_cast<uint8_t>(type) < _textures.size())
	{
 		return _textures[static_cast<uint8_t>(MaterialTextureType::DiffuseTexture)];
	}

	return nullptr;
}

Ref<mrs::Material> mrs::Material::Create(const std::string& base_template_name, const std::string& alias, const std::string& texture_name)
{
	return Create(VulkanAssetManager::Instance().FindEffectTemplate(base_template_name), alias, texture_name);
}

Ref<mrs::Material> mrs::Material::Create(Ref<EffectTemplate> base_template, const std::string& alias, const std::string& texture_name)
{
	ResourceManager::Get()._materials[alias] = std::make_shared<Material>();
	ResourceManager::Get()._materials[alias]->_name = alias;
	ResourceManager::Get()._materials[alias]->_base_template = base_template;
	ResourceManager::Get()._materials[alias]->_textures[static_cast<uint8_t>(MaterialTextureType::DiffuseTexture)] = Texture::Get(texture_name);

	return ResourceManager::Get()._materials[alias];
}

Ref<mrs::Material> mrs::Material::Get(const std::string& alias)
{
	auto it = ResourceManager::Get()._materials.find(alias);

	if (it != ResourceManager::Get()._materials.end()) 
	{
		return it->second;
	}

	return nullptr;
}
