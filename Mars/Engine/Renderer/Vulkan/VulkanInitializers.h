#ifndef VULKANINITIALIZERS_H
#define VULKANINITIALIZERS_H

#pragma once

#include <vulkan/vulkan.h>

namespace vkinit {
	VkCommandPoolCreateInfo command_pool_create_info(uint32_t queue_family_index, VkCommandPoolCreateFlags create_flags = 0);
	VkCommandBufferAllocateInfo command_buffer_alloc_info(VkCommandPool cmd_pool, VkCommandBufferLevel level =  VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags usage);
	VkRenderPassBeginInfo render_pass_begin_info(VkFramebuffer frame_buffer, VkRenderPass render_pass, VkRect2D& render_area, VkClearValue* clear_value, uint32_t clear_value_count);
}


#endif