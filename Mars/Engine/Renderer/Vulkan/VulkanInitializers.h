#ifndef VULKANINITIALIZERS_H
#define VULKANINITIALIZERS_H

#pragma once

#include <vulkan/vulkan.h>

namespace vkinit {

	// Command infos
	VkCommandPoolCreateInfo command_pool_create_info(uint32_t queue_family_index, VkCommandPoolCreateFlags create_flags = 0);
	VkCommandBufferAllocateInfo command_buffer_alloc_info(VkCommandPool cmd_pool, VkCommandBufferLevel level =  VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags usage);
	VkRenderPassBeginInfo render_pass_begin_info(VkFramebuffer frame_buffer, VkRenderPass render_pass, VkRect2D& render_area, VkClearValue* clear_value, uint32_t clear_value_count);

	// Pipelines configuration
	VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule, const char* entrypoint = "main");
	VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info();
	VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info(VkPrimitiveTopology topology);
	VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info(VkPolygonMode polygonMode);
	VkPipelineMultisampleStateCreateInfo pipeline_mulitisample_state_create_info();
	VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state();
	VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_create_info(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);
	VkPipelineLayoutCreateInfo pipeline_layout_create_info();

	// Resources
	VkImageCreateInfo image_create_info(VkFormat format, VkExtent3D extent, VkImageUsageFlags usage);
	VkImageViewCreateInfo image_view_create_info(VkImage image, VkFormat format, VkImageAspectFlags aspect);

	VkSamplerCreateInfo sampler_create_info(VkFilter filter, VkSamplerAddressMode sampler_address_mode);
}


#endif