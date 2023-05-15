#ifndef VULKANUTILS_H
#define VULKANUTILS_H

#pragma once

#include <iostream>
#include <functional>
#include <deque>
#include <vector>

#include <vulkan/vulkan.h>
#include <unordered_map>

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

	// Handles the allocation descriptors and descriptor pools
	class DescriptorAllocator
	{
	public:

		struct PoolSizes {
			std::vector<std::pair<VkDescriptorType,float>> sizes =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
			};
		};

		void Init(VkDevice device);
		void CleanUp();

		// Allocates single descriptor set
		bool Allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout);

		// Gets free descripor pool if available and allocates new ones when needed
		// Pools are created with a factor of n * 1000 descriptor set sizes
		VkDescriptorPool GetPool();
		
		// Rests all pools and stores in to free pools vector
		void Reset();

		// Device handle
		VkDevice _device_h;
	private:
		VkDescriptorPool _current_pool = VK_NULL_HANDLE;

		PoolSizes descriptor_pool_sizes = {};
		std::vector<VkDescriptorPool> _free_pools;
		std::vector<VkDescriptorPool> _used_pools;
	};

	// Cache to store descriptor set layouts
	class DescriptorLayoutCache
	{
	public:
		void Init(VkDevice device);
		void Clear();

		// Info and Identifier for descriptor set layouts in cache map
		struct DescriptorLayoutInfo
		{
			// DescriptorSets are indentified by bindings
			std::vector<VkDescriptorSetLayoutBinding> bindings;

			bool operator==(const DescriptorLayoutInfo& other) const;

			size_t Hash() const;
		};

		VkDescriptorSetLayout CreateDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo* info);
	private:

		// Hash for Descriptor Set Layout
		struct DescriptorLayoutHash
		{
			std::size_t operator()(const DescriptorLayoutInfo& k) const {
				return k.Hash();
			}
		};

		std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> _cache = {};
		VkDevice _device_h;
	};

	// Handles Allococation of descriptor pools, for a single set, set layout and its bindings
	// then binds buffer/images to descriptors
	class DescriptorBuilder
	{
	public:

		// Creates and returns builder with reference to cache and descriptor allocator
		static DescriptorBuilder Begin(DescriptorLayoutCache* layout_cache, DescriptorAllocator* _descriptor_allocator) {
			DescriptorBuilder builder = {};
			builder._cache = layout_cache;
			builder._allocator = _descriptor_allocator;

			return builder;
		};

		// Queues buffer to descriptor binding
		DescriptorBuilder& BindBuffer(uint32_t binding, VkDescriptorBufferInfo* buffer_info, VkDescriptorType type, VkShaderStageFlags shader_stage);

		// Queues image to descriptor binding
		DescriptorBuilder& BindImage(uint32_t binding, VkDescriptorImageInfo* image_info, VkDescriptorType type, VkShaderStageFlags shader_stage);

		bool Build(VkDescriptorSet* set, VkDescriptorSetLayout* layout);
	private:
		std::vector<VkWriteDescriptorSet> _writes;
		std::vector<VkDescriptorSetLayoutBinding> _bindings;

		DescriptorLayoutCache* _cache;
		DescriptorAllocator* _allocator;
	};

	// Creates descriptor pool given sizes. Should only be used by DescriptorAllocator type
	VkDescriptorPool create_pool(VkDevice device, const DescriptorAllocator::PoolSizes& pool_sizes, uint32_t count, VkDescriptorPoolCreateFlags create_flags);
}
#endif