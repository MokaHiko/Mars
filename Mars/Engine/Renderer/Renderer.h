#ifndef RENDERER_H
#define RENDERER_H

#pragma once 

#include <memory>
#include <vector>
#include <functional>

#include <vulkan/vulkan.h>

#include "Core/Window.h"
#include "Vulkan/VulkanUtils.h"
#include "Vulkan/VulkanStructures.h"

#include "Vulkan/VulkanMesh.h"

namespace mrs {

	struct GraphicsSettings
	{
		bool vsync = true;
	};

	class Renderer
	{
	public:
		Renderer(const std::shared_ptr<Window> window);
		~Renderer();

		virtual void Init();
		virtual void Update();
		virtual void Shutdown();

		void UploadResources();
	private:
		const std::shared_ptr<Window> _window;
	public:
		// ~ Client Application Level
		std::shared_ptr<Mesh> my_mesh;

		struct StaticObjectData
		{
			glm::vec4 color;
			glm::mat4 model_matrix;
		};

		// Global frame descriptors
		VkDescriptorSet global_descriptor_set;
		VkDescriptorSetLayout global_descriptor_set_layout;
		AllocatedBuffer global_descriptor_buffer;
	public:
		// Returns whether or not shader module was created succesefully
		bool LoadShaderModule(const char* path, VkShaderModule* module);

		// Used for immedaite time and blocking execution of commands 
		void ImmediateSubmit(std::function<void(VkCommandBuffer)>&& fn);

		// Creates and allocates buffer with given size
		AllocatedBuffer CreateBuffer(size_t size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage, VkMemoryPropertyFlags memory_props = 0);
	private:
		// Creates swapchain and gets handle to image s/views
		void InitSwapchain();

		// Creates command pools and command buffers
		void InitCommands();

		// Creates a default render_pass;
		void InitDefaultRenderPass();

		// Creates framebuffer that points the attachments in render pass to images in swapchain
		void InitFramebuffers();

		// Creates all the fences and semaphores
		void InitSyncStructures();

		// Creates graphics pipelines
		void InitPipelines();

		// Creates and allocates descriptors 
		void InitDescriptors();
	private:
		inline uint32_t GetCurrentFrame() const { return _frame_count % frame_overlaps; }

		// Pads size to be compatible with minimum unifrom buffer alignment of physical device
		size_t PadToUniformBufferSize(size_t original_size);

		// Upload mesh to GPU via immediate command buffers
		void UploadMesh(std::shared_ptr<Mesh> mesh);
	private:
		// Vulkan Structures
		VkInstance _instance = {};
		VkSurfaceKHR _surface = {};
		VkDebugUtilsMessengerEXT _debug_messenger = {};

		VkSwapchainKHR _swapchain = {};
		VkFormat _swapchain_image_format = {};
		std::vector<VkImage> _swapchain_images = {};
		std::vector<VkImageView> _swapchain_image_views = {};

		AllocatedImage _depth_image = {};
		VkFormat _depth_image_format = {};
		VkImageView _depth_image_view = {};

		VkRenderPass _render_pass = {};
		std::vector<VkFramebuffer> _framebuffers;

		VkPipeline _default_pipeline;
		VkPipelineLayout _default_pipeline_layout;

		VulkanDevice _device = {};
		VulkanQueues _queues = {};
		VkPhysicalDeviceProperties _physical_device_props = {};
		VulkanQueueFamilyIndices _queue_indices = {};

		// Number of frame contexts
		static const uint64_t time_out = 1000000000;
		static const uint32_t frame_overlaps = 2;
		VulkanFrameContext _frame_data[frame_overlaps];

		// Main upload struct
		VulkanUploadContext _upload_context = {};

		vkutil::DeletionQueue _deletion_queue;
		uint32_t _frame_count = 0;

	private:
		// ~Vulkan resource management

		// Main gpu upload resource allocator
		VmaAllocator _allocator;

		// Desccriptor allocator and layout cache
		std::shared_ptr<vkutil::DescriptorAllocator> _descriptor_allocator;
		std::shared_ptr<vkutil::DescriptorLayoutCache> _descriptor_layout_cache;
	};
}

#endif