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
		
		bool receive_shadows = true;
	};

	class Material
	{
	public:
		std::string& GetMaterialName() {return _material_name;}
		std::string GetMaterialName() const {return _material_name;}

		std::string& GetDiffuseTextureName() {return _diffuse_texture_path;}
		std::string GetDiffuseTextureName() const {return _diffuse_texture_path;}

		const glm::vec4& GetAlbedoColor() const {return _data.diffuse_color;}
		glm::vec4& GetAlbedoColor() {return _data.diffuse_color;}

		const bool& GetShadowFlag() const {return _data.receive_shadows;}
		bool& GetShadowFlag() {return _data.receive_shadows;}

		// Returns index of material in global gpu materials buffer
		const uint32_t GetMaterialIndex() const { return _material_index;}
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
		friend class VulkanAssetManager;
		friend class ResourceManager;

		std::string _material_name;
		std::string _diffuse_texture_path;

		MaterialData _data = {};
		VkPipeline _render_pipeline = VK_NULL_HANDLE;

		// Index of material in materials buffer
		uint32_t _material_index;
	};
}
#endif