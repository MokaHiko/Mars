#include "VulkanTexture.h"

#include <boop.h>

#include "Core/Memory.h"
#include "Core/ResourceManager.h"

namespace mrs {
	Texture::Texture()
	{
	}
	Texture::~Texture()
	{
	}

	Ref<Texture> Texture::LoadFromAsset(const std::string& path, const std::string& alias)
	{
		std::string name;
		if (!alias.empty()) {
			name = alias;
		}
		else {
			name = path;
		}

		if (Texture::Get(name) != nullptr) {
			return Texture::Get(name);
		}

		auto texture = std::make_shared<Texture>();
		texture->_name = name;
		ResourceManager::Get()._textures[name] = texture;

		// Load texture asset file
		boop::AssetFile asset = {};
		boop::load(path.c_str(), asset);

		// Unpack data into texture
		boop::TextureInfo texture_info = boop::read_texture_info(&asset);
		texture->_width = texture_info.pixel_size[0];
		texture->_height = texture_info.pixel_size[1];

		if (texture_info.format == boop::TextureFormat::RGBA8) {
			texture->_format = VK_FORMAT_R8G8B8A8_SRGB;
		}

		texture->pixel_data.resize(static_cast<size_t>(texture_info.texture_size));
		boop::unpack_texture(&texture_info, asset.raw_data.data(), asset.raw_data.size(), texture->pixel_data.data());

		return texture;
	}

	Ref<Texture> Texture::Get(const std::string& path)
	{
		auto it = ResourceManager::Get()._textures.find(path);

		if (it != ResourceManager::Get()._textures.end()) {
			return it->second;
		}

		return nullptr;
	}

    void Texture::SetSamplerType(SamplerType type)
	{
		// TODO: ReUpload After Sampler Change
		_sampler_type = type;
	}
}

