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
};

#endif