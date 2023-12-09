#ifndef VULKANASSETMANAGER_H
#define VULKANASSETMANAGER_H

#pragma once

#include "Core/Memory.h"

#include "VulkanUtils.h"
#include "VulkanStructures.h"

#include "VulkanMesh.h"
#include "VulkanTexture.h"
#include "VulkanMaterial.h"

namespace mrs
{
    class Renderer;
    class Shader;

    // Manages runtime allocatiion of vulkan assets (textures, meshes)
    class VulkanAssetManager
    {
    public:
        static VulkanAssetManager& Instance()
        {
            if(!_instance)
            {
                _instance = CreateRef<VulkanAssetManager>();
                return *_instance.get();
            }

            return *_instance.get();
        }

        void Init(Renderer* renderer);
        void Shutdown();

        VulkanAssetManager();
        ~VulkanAssetManager();

        Ref<EffectTemplate> FindEffectTemplate(const std::string& name);
        Ref<EffectTemplate> CreateEffectTemplate(const std::vector<ShaderEffect*>& effects, const std::string name);

        // Creates
	    void UploadMesh(Ref<Mesh> mesh);

        void UploadMaterial(Ref<Material> material);

        // Creates VkImage and VkImageView from data stored in texture 
        void UploadTexture(Ref<Texture> texture);

        // Uploads Sprites
        void UploadSprite(Ref<Sprite> sprite);

		// Returns whether or not shader module was created succesefully
		bool LoadShaderModule(const char *path, VkShaderModule *module);

	    const Ref<Shader> LoadShader(const char* path, VkShaderStageFlagBits stage);

        const VkSampler LinearImageSampler() const {return _linear_image_sampler;}
        const VkSampler NearestImageSampler() const {return _nearest_image_sampler;}

        // TODO: Remove as each material will have custom descriptor sets descriptor sets
        // Returns the storage that contains vulkan materials
		const VkDescriptorSetLayout MaterialDescriptorSetLayout() const {return _material_descriptor_set_layout;}
    private:
        // Materials descriptor set
		VkDescriptorSetLayout _material_descriptor_set_layout = VK_NULL_HANDLE;

		VkSampler _linear_image_sampler = VK_NULL_HANDLE;
		VkSampler _nearest_image_sampler = VK_NULL_HANDLE;

        // Reference to renderer
        Renderer* _renderer = nullptr;

        // Material effect templates
        std::unordered_map<std::string, Ref<EffectTemplate>> _effect_templates;

        static Ref<VulkanAssetManager> _instance;
    };

}
#endif