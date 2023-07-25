#ifndef VULKANASSETMANAGER_H
#define VULKANASSETMANAGER_H

#pragma once

#include "VulkanUtils.h"
#include "VulkanStructures.h"

#include "VulkanMesh.h"
#include "VulkanTexture.h"
#include "VulkanMaterial.h"

namespace mrs
{
    // Manages Allocating Textures & Meshes to GPU
    class Renderer;
    class VulkanAssetManager
    {
    public:
        VulkanAssetManager(Renderer* renderer);
        ~VulkanAssetManager();

        // Createsj
        void UploadMaterial(std::shared_ptr<Material> material);

        // Creates VkImage and VkImageView from data stored in texture 
        void UploadTexture(std::shared_ptr<Texture> texture);

        const VkSampler GetLinearImageSampler() const {return _linear_image_sampler;}
        const VkSampler GetNearestImageSampler() const {return _nearest_image_sampler;}

        // Returns the storage that contains vulkan materials
        const AllocatedBuffer& GetMaterialsBuffer() const {return _materials_descriptor_buffer;}
		const VkDescriptorSetLayout GetMaterialDescriptorSetLayout() const {return _material_descriptor_set_layout;}

    private:
        // Materials descriptor set
		VkDescriptorSetLayout _material_descriptor_set_layout;
		AllocatedBuffer _materials_descriptor_buffer;
        uint32_t material_insert_index = 0;

		VkSampler _linear_image_sampler;
		VkSampler _nearest_image_sampler;

        // Reference to renderer
        Renderer* _renderer;
    };

}
#endif