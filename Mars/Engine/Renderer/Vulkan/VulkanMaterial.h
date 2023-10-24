#ifndef VULKANMATERIAL_H
#define VULKANMATERIAL_H

#pragma once

#include "VulkanStructures.h"
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

	class Shader
	{
	public:
		std::string name;
		std::vector<char> byte_code;

		VkShaderModule shader_module;
		VkShaderStageFlagBits stage;
	};

	struct ShaderEffect
	{
		IRenderPipeline* render_pipeline;
		std::vector<VkDescriptorSet> descriptor_sets;
	};

	// Base effect a material is built from
	struct EffectTemplate
	{
		std::vector<ShaderEffect*> shader_effects;
		std::string name;
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
		void SetTexture(MaterialTextureType type, Ref<Texture> texture);

		static Ref<Material> Create(Ref<EffectTemplate> base_template, Ref<Texture> texture, const std::string& alias = "");
		static Ref<Material> Create(Ref<EffectTemplate> base_template, const std::string& texture_name = "default_texture", const std::string& alias = "");
		static Ref<Material> Create(const std::string& base_template_name, const std::string& texture_name = "default_texture", const std::string& alias = "");

		static Ref<Material> Get(const std::string& alias);

		Ref<EffectTemplate> BaseTemplate() { return _base_template; }
	public:
		const VkDescriptorSet DescriptorSet() const { return _material_descriptor_set; }
		VkDescriptorSet& DescriptorSet() { return _material_descriptor_set; }

		AllocatedBuffer& Buffer() { return _material_buffer; }
		const AllocatedBuffer& Buffer() const { return _material_buffer; }

		MaterialData& Data() { return _data; }
		const MaterialData& Data() const { return _data; }
	private:

		friend class VulkanAssetManager;
		friend class ResourceManager;

		Ref<EffectTemplate> _base_template;

		AllocatedBuffer _material_buffer = {};
		VkDescriptorSet _material_descriptor_set = VK_NULL_HANDLE;

		std::string _name;
		std::array<Ref<Texture>, 4> _textures = {}; 
		MaterialData _data = {};
	};
}
#endif