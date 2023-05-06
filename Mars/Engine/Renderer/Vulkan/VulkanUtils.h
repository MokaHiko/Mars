#ifndef VULKANUTILS_H
#define VULKANUTILS_H

#pragma once

#include <iostream>
#include <functional>
#include <deque>
#include <vector>

#include <vulkan/vulkan.h>

#define VK_CHECK(x)                                                     \
	do                                                                  \
	{                                                                   \
		VkResult err = x;                                               \
		if (err)                                                        \
		{                                                               \
			std::cout << "Detected Vulkan error: " << err << std::endl; \
			abort();                                                    \
		}                                                               \
	} while (0)

namespace vkutil {

	// Queue that keeps clean up functions
	struct DeletionQueue
	{
		// Pushes and takes ownership of callback in deletion queue for clean up in FIFO order
		void Push(std::function<void()>&& fn)
		{
			clean_functions.push_back(fn);
		}

		// Calls all clean up functions in FIFO order
		void Flush()
		{
			for (auto it = clean_functions.rbegin(); it != clean_functions.rend(); it++)
			{
				(*it)();
			}

			clean_functions.clear();
		}
	private:
		std::deque<std::function<void()>> clean_functions = {};
	};

	// Creates Graphics Pipeline using the builder pattern
	class PipelineBuilder
	{
	public:
		std::vector<VkPipelineShaderStageCreateInfo> _shader_stages = {};
		VkPipelineVertexInputStateCreateInfo _vertex_input_info = {};
		VkPipelineInputAssemblyStateCreateInfo _input_assembly = {};
		VkPipelineRasterizationStateCreateInfo _rasterizer = {};
		VkPipelineMultisampleStateCreateInfo _multisampling = {};
		VkPipelineColorBlendAttachmentState _color_blend_attachment = {};
		VkPipelineDepthStencilStateCreateInfo _depth_stencil = {};
		VkPipelineLayout _pipeline_layout = {};

		VkViewport _viewport = {};
		VkRect2D _scissor = {};

	public:
		VkPipeline Build(VkDevice device, VkRenderPass renderPass);
	};
}
#endif