#ifndef RENDERER_H
#define RENDERER_H

#pragma once

#include <limits>
#include <vulkan/vulkan.h>

#include "Core/Window.h"
#include "Vulkan/VulkanUtils.h"
#include "Vulkan/VulkanStructures.h"
#include "Vulkan/VulkanAssetManager.h"

#include "Camera.h"
#include "ECS/Scene.h"

namespace mrs {
	// Time out for render commands and fences 
	static const uint64_t time_out = std::numeric_limits<uint64_t>::max();

	// Maximum allowable frames in flight
	static const uint32_t frame_overlaps = 2;

	struct GraphicsSettings
	{
		bool vsync = true;
		bool tesselation = true;
	};

	struct RendererInfo
	{
		Ref<Window> window;
		GraphicsSettings graphics_settings = {};
		uint32_t max_objects = 10000;
		uint32_t max_materials = 100;
		
		uint32_t max_dir_lights = 10;
		uint32_t max_point_lights = 100;
	};

	// Common object data unique to each entity (Model matrix, ...)
	struct ObjectData
	{
		glm::vec4 color{ 1.0f };
		glm::mat4 model_matrix{ 1.0f };
	};

	struct DirectionalLight
	{
		glm::mat4 view_proj;
		glm::vec4 Direction;

		glm::vec4 Ambient = glm::vec4(1.0f);
		glm::vec4 Diffuse = glm::vec4(1.0f);
		glm::vec4 Specular = glm::vec4(1.0f);
	};

	// Global shared scene data
	struct GlobalDescriptorData
	{
		glm::mat4 view;
		glm::mat4 view_proj;
		glm::vec4 camera_position;

		uint32_t n_dir_lights;
	};

	// Main vulkan state wrapper that manages the lifetime of vulkan resources and execution of renderpasses and renderpipelines
	class Renderer
	{
	public:
		Renderer(RendererInfo &info);
		~Renderer();

		void Init();
		void Shutdown();

        void WaitForFences();

		// Starts the current frame and the recording of the command buffer
		void Begin(Scene *scene);

		// Starts mesh render pass
		void MeshPassStart(VkCommandBuffer cmd, VkFramebuffer frame_buffer, VkRenderPass render_pass);

		// Ends a mesh pass
		void MeshPassEnd(VkCommandBuffer cmd);

		// Begins the swapchain render pass
		void MainPassStart(VkCommandBuffer cmd);

		// Ends the swapchain render pass
		void MainPassEnd(VkCommandBuffer cmd);

		// Ends the command buffer recording and submits to queue
		void End();

		// TODO: Remove and move to render pipeline layer
		// Uploads resources stored in resource manager
		void UploadResources();

		// Gets handle to main renderpass camera
		Camera *GetCamera() { return _camera; }

		// Sets renderer camera
		void SetCamera(Camera *camera) { _camera = camera; }

		// Adds semaphore for graphics queue to wait during submission
		void PushGraphicsSemaphore(VkPipelineStageFlags wait_stage,  VkSemaphore semaphore) {_graphics_wait_stages.push_back(wait_stage), _graphics_wait_semaphores.push_back(semaphore);}
	public:
		std::vector<AllocatedBuffer>& GlobalBuffers();
		std::vector<AllocatedBuffer>& ObjectBuffers();
		std::vector<AllocatedBuffer>& DirLightBuffers();

		vkutil::DescriptorAllocator* DescriptorAllocator() {return _descriptor_allocator.get();}
		vkutil::DescriptorLayoutCache* DescriptorLayoutCache() {return _descriptor_layout_cache.get();}
	public:
		void InitOffScreenAttachments();
		void InitOffScreenRenderPass();

		void InitOffScreenFramebuffers();

		VkRenderPass _offscreen_render_pass = {};

        // Offscreen frame buffers
        const VkFormat _offscreen_framebuffer_format = VK_FORMAT_R8G8B8A8_UNORM;
        AllocatedImage _offscreen_image; 
        VkImageView _offscreen_images_view; 
        VkFramebuffer _offscreen_framebuffer;

        // Offscreen depth attachments
		AllocatedImage _offscreen_depth_image = {};
		VkFormat _offscreen_depth_image_format = {};
		VkImageView _offscreen_depth_image_view = {};
	public:
		const VkInstance Instance() const { return _instance; }

		const VkFormat SwapchainImageFormat() const {return _swapchain_image_format;}
		VkImageView OffScreenImageView() const { return _offscreen_images_view; }
		const std::vector<VkImage> SwapchainImages() const {return _swapchain_images;}
		const std::vector<VkImageView> SwapchainImageViews() const {return _swapchain_image_views;}
		const VkRenderPass SwapchainRenderPass() const {return _render_pass;}

		const uint32_t CurrentFrame() const { return _frame_count % frame_overlaps; }
		const uint32_t CurrentSwapChainImage() const { return _current_swapchain_image; }

		const VkFramebuffer CurrentFrameBuffer() const { return _framebuffers[_current_swapchain_image]; }
		const VulkanFrameContext &CurrentFrameData() const { return _frame_data[_frame_count % frame_overlaps]; }

		VulkanDevice &Device() { return _device; }
		vkutil::DeletionQueue &DeletionQueue() { return _deletion_queue; }

		VmaAllocator &Allocator() { return _allocator; }
		VulkanQueues &Queues() { return _queues; }
		VulkanQueueFamilyIndices &GetQueueIndices() { return _queue_indices; }

		AllocatedBuffer CreateBuffer(size_t size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage, VkMemoryPropertyFlags memory_props = 0);

		size_t PadToUniformBufferSize(size_t original_size);
		size_t PadToStorageBufferSize(size_t original_size);

		void ImmediateSubmit(std::function<void(VkCommandBuffer)> &&fn);
	private:
		void InitVulkan();
		void InitSwapchain();
		void InitCommands();
		void InitDefaultRenderPass();
		void InitFramebuffers();
		void InitSyncStructures();

		void InitGlobalDescriptors();
		void UpdateGlobalDescriptors(Scene *scene, uint32_t frame_index);
	public:
		// General renderer info
		RendererInfo _info = {};

		// Handle to window being rendered to
		const Ref<Window> _window;

		// Cameras perspective to render to
		Camera *_camera = nullptr;

		// Global descriptor
		std::vector<VkDescriptorSet> _global_descriptor_sets;
		std::vector<AllocatedBuffer> _global_descriptor_buffers;
		VkDescriptorSetLayout _global_descriptor_set_layout;

		// Directional lights descriptors
		std::vector<VkDescriptorSet> _dir_light_descriptor_sets;
		std::vector<AllocatedBuffer> _dir_light_descriptor_buffers;

		// Global Object descriptors (per frame)
		std::vector<VkDescriptorSet> _object_descriptor_sets;
		std::vector<AllocatedBuffer> _object_descriptor_buffers;

		VkDescriptorSetLayout _object_descriptor_set_layout;

		VulkanFrameContext _frame_data[frame_overlaps];
	private:
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
		VkPhysicalDeviceProperties _physical_device_props = {};
		VulkanQueueFamilyIndices _queue_indices = {};

		uint32_t _current_swapchain_image = -1;

		// Upload context for immediate submits
		VulkanUploadContext _upload_context = {};

		vkutil::DeletionQueue _deletion_queue;

		uint32_t _frame_count = 0;

		// Main gpu upload resource allocator
		VmaAllocator _allocator = {};

		// Descriptor allocator and layout cache
		Ref<vkutil::DescriptorAllocator> _descriptor_allocator;
		Ref<vkutil::DescriptorLayoutCache> _descriptor_layout_cache;

		// Semaphores the graphics queue will wait for during submission
		std::vector<VkPipelineStageFlags> _graphics_wait_stages = {};
		std::vector<VkSemaphore> _graphics_wait_semaphores = {};
	};
}

#endif