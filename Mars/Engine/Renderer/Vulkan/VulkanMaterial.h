#ifndef VULKANMATERIAL_H
#define VULKANMATERIAL_H

#pragma once

#include <vulkan/vulkan.h>
#include "Core/ResourceManager.h"

namespace mrs {

	class Material
	{
	public:
		VkDescriptorSet texture_set = VK_NULL_HANDLE;
		VkPipeline pipeline;
		VkDescriptorSetLayout layout;

		static std::shared_ptr<Material> Create(const std::string& alias, const std::string& texture_name = "default_texture")
		{
			ResourceManager::Get()._materials[alias] = std::make_shared<Material>();
			ResourceManager::Get()._materials[alias]->diffuse_texture_path = texture_name;

			ResourceManager::Get()._materials[alias]->_material_name = alias;
			return ResourceManager::Get()._materials[alias];
		}

		static std::shared_ptr<Material> Get(const std::string& alias)
		{
			auto it = ResourceManager::Get()._materials.find(alias);

			if (it != ResourceManager::Get()._materials.end()) {
				return it->second;
			}

			return nullptr;
		}

		std::string _material_name;
		std::string diffuse_texture_path;
	};
}
#endif