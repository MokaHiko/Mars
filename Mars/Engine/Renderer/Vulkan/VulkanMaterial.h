#ifndef VULKANMATERIAL_H
#define VULKANMATERIAL_H

#pragma once

#include <vulkan/vulkan.h>
#include "Core/ResourceManager.h"
#include <glm/glm.hpp>

namespace mrs {
	// Data passed as descriptor set
	struct MaterialData
	{
		// Albedo
		glm::vec4 diffuse_color{1.0f};

		// Metallic
		float metallic = 0.5f;
		float specular = 0.5f;
		int texture_channel = 2;
	};

	class Material
	{
	public:
		std::string& MaterialName() {return _material_name;}
		std::string MaterialName() const {return _material_name;}

		const glm::vec4& AlbedoColor() const {return _color;}
		glm::vec4& AlbedoColor() {return _color;}
	public:
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkDescriptorSetLayout layout = VK_NULL_HANDLE;
		VkDescriptorSet material_descriptor_set= VK_NULL_HANDLE;

		static std::shared_ptr<Material> Create(const std::string& alias, const std::string& texture_name = "default_texture")
		{
			ResourceManager::Get()._materials[alias] = std::make_shared<Material>();
			ResourceManager::Get()._materials[alias]->_diffuse_texture_path = texture_name;

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

	private:
		friend class Renderer;
		friend class ResourceManager;

		std::string _material_name;
		std::string _diffuse_texture_path;

		// Albedo
		glm::vec4 _color{1.0f};

		// Metallic
		float _metallic = 0.0f;
		float _specular = 0.0f;
	};
}
#endif