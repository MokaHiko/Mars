#ifndef VULKANMATERIAL_H
#define VULKANMATERIAL_H

#pragma once

#include <vulkan/vulkan.h>
#include "Core/ResourceManager.h"
#include <glm/glm.hpp>

namespace mrs {
	class IRenderPipeline;

	enum class MaterialTextureType : uint8_t
	{
		DiffuseTexture = 0,
		SpecularTexture = 1,
		Roughness = 2,
	};

	struct ShaderEffect
	{
		IRenderPipeline* render_pipeline;
		VkDescriptorSetLayout descriptor_set_layout;
		VkDescriptorSet descriptor_set;
	};

	struct EffectTemplate
	{
		std::vector<ShaderEffect*> shader_effects;
	};

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
		std::string& Name() {return _name;}
		const std::string Name() const {return _name;}
		
		Ref<Texture> MainTexture();
		Ref<Texture> GetTexture(MaterialTextureType type);

		const uint32_t MaterialIndex() const { return _material_index;}

		static Ref<Material> Create(const std::string& base_template_name, const std::string& alias, const std::string& texture_name = "default_texture");
		static Ref<Material> Create(Ref<EffectTemplate> base_template, const std::string& alias, const std::string& texture_name = "default_texture");
		static Ref<Material> Get(const std::string& alias);

		Ref<EffectTemplate> BaseTemplate() { return _base_template; }
	public:
		const VkDescriptorSet DescriptorSet() const { return _material_descriptor_set; }
		VkDescriptorSet& DescriptorSet() { return _material_descriptor_set; }
	private:

		friend class VulkanAssetManager;
		friend class ResourceManager;

		Ref<EffectTemplate> _base_template;
		VkDescriptorSet _material_descriptor_set = VK_NULL_HANDLE;

		std::string _name;
		std::array<Ref<Texture>, 4> _textures = {}; 

		MaterialData _data = {};

		// Index of material in materials buffer
		uint32_t _material_index;
	};
}
#endif