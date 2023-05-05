#ifndef RENDERER_H
#define RENDERER_H

#pragma once 

#include <memory>
#include <vulkan/vulkan.h>
#include <vector>

#include "Core/Window.h"
#include "Vulkan/VulkanUtils.h"
#include "Vulkan/VulkanStructures.h"

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
	private:
		const std::shared_ptr<Window> _window;
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
	private:
		inline uint32_t GetCurrentFrame() const { return _frame_count % frame_overlaps; }
	private:
		// Vulkan Structures
		VkInstance _instance = {};
		VkSurfaceKHR _surface = {};
		VkDebugUtilsMessengerEXT _debug_messenger = {};

		VkSwapchainKHR _swapchain = {};
		VkFormat _swapchain_image_format = {};
		std::vector<VkImage> _swapchain_images = {};
		std::vector<VkImageView> _swapchain_image_views = {};

		VkRenderPass _render_pass = {};

		std::vector<VkFramebuffer> _framebuffers;

		VulkanDevice _device = {};
		VulkanQueues _queues = {};
		VulkanQueueFamilyIndices _queue_indices = {};
		
		// Number of frame contexts
		static const uint32_t frame_overlaps = 2;
		VulkanFrameContext _frame_data[frame_overlaps];

		// Main upload struct
		VulkanUploadContext _upload_context = {};

		vkutil::DeletionQueue _deletion_queue;

		uint32_t _frame_count = 0;
	};
}

#endif