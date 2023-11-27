#ifndef VULKANSTRUCTURES_H
#define VULKANSTRUCTURES_H

#pragma once

#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

struct VulkanDevice
{
	VkDevice device;
	VkPhysicalDevice physical_device;
};

struct VulkanQueueFamilyIndices
{
	uint32_t graphics;
	uint32_t present;
	uint32_t transfer;
	uint32_t compute;
};

struct VulkanQueues
{
	VkQueue graphics;
	VkQueue present;
	VkQueue transfer;
	VkQueue compute;
};

// Data needed for direct uploading data to the gpu
struct VulkanUploadContext
{
	// Fence that blocks while uploading
	VkFence upload_fence;
	VkCommandPool command_pool;
	VkCommandBuffer command_buffer;
};


// A struct that holds a handle to buffer and its allocation
struct AllocatedBuffer
{
	VkBuffer buffer;
	VmaAllocation allocation;
};

// A struct that holds a handle to image and a allocation
struct AllocatedImage
{
	VkImage image;
	VmaAllocation allocation;
};

// Objects that need to handled for each frame
struct VulkanFrameContext
{
	VkFence render_fence;

	VkSemaphore present_semaphore;
	VkSemaphore render_semaphore;

	// Main command pool for frame
	VkCommandPool command_pool;
	VkCommandBuffer command_buffer;

	VkCommandPool compute_command_pool;
	VkCommandBuffer compute_command_buffer;
};

struct VulkanDescriptorSet
{
	uint32_t set = -1;
	std::unordered_map<uint32_t, VkDescriptorType> bindings;
	std::unordered_map<uint32_t, std::string> binding_names;
	VkShaderStageFlagBits shader_stage = (VkShaderStageFlagBits)(0);
	VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;

	void AddBinding(uint32_t binding, const std::string& name, VkDescriptorType type, VkShaderStageFlagBits stage)
	{
		bindings[binding] = type;
		binding_names[binding] = name;

		shader_stage = static_cast<VkShaderStageFlagBits>(shader_stage | stage);
	}

	const std::string& BindingName(uint32_t binding) const
	{
		return binding_names.at(binding);
	}
};

#endif