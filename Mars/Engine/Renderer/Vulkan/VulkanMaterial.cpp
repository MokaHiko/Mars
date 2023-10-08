#include "VulkanMaterial.h"

#include "Core/Application.h"
#include "Renderer/RenderPipelineLayers/IRenderPipelineLayer.h"

Ref<mrs::Texture> mrs::Material::MainTexture()
{
	return GetTexture(MaterialTextureType::DiffuseTexture);
}

Ref<mrs::Texture> mrs::Material::GetTexture(MaterialTextureType type)
{
	uint32_t index = static_cast<uint8_t>(type);
	if(index < _textures.size())
	{
 		return _textures[index];
	}

	return nullptr;
}

void mrs::Material::SetTexture(MaterialTextureType type, Ref<Texture> texture)
{
	uint32_t index = static_cast<uint8_t>(type);
	if(index < _textures.size())
	{
 		_textures[index] = texture;
	}
}

Ref<mrs::Material> mrs::Material::Create(Ref<EffectTemplate> base_template, Ref<Texture> texture, const std::string& alias)
{
	ResourceManager::Get()._materials[alias] = std::make_shared<Material>();
	ResourceManager::Get()._materials[alias]->_name = alias;
	ResourceManager::Get()._materials[alias]->_base_template = base_template;

	ResourceManager::Get()._materials[alias]->_textures[static_cast<uint8_t>(MaterialTextureType::DiffuseTexture)] = texture;
	ResourceManager::Get()._materials[alias]->_textures[static_cast<uint8_t>(MaterialTextureType::SpecularTexture)] = Texture::Get("default");

	return ResourceManager::Get()._materials[alias];
}

Ref<mrs::Material> mrs::Material::Create(const std::string& base_template_name, const std::string& texture_name, const std::string& alias)
{
	return Create(VulkanAssetManager::Instance().FindEffectTemplate(base_template_name), texture_name, alias);
}

Ref<mrs::Material> mrs::Material::Create(Ref<EffectTemplate> base_template, const std::string& texture_name, const std::string& alias)
{
	return Create(base_template, Texture::Get(texture_name), alias);
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
