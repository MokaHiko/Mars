#ifndef RENDERER_H
#define RENDERER_H

#pragma once

#include <limits>
#include <vulkan/vulkan.h>

#include "Core/Window.h"
#include "Vulkan/VulkanUtils.h"
#include "Vulkan/VulkanStructures.h"

#include "Camera.h"
#include "ECS/Scene.h"

#include "Vulkan/VulkanAssetManager.h"

namespace mrs
{
	// Time out for render commands and fences 
	static const uint64_t time_out = std::numeric_limits<uint64_t>::max();

	// Maximum allowable frames in flight
	static const uint32_t frame_overlaps = 2;

	struct GraphicsSettings
	{
		bool vsync = true;
	};

	struct RendererInfo
	{
		std::shared_ptr<Window> window;
		GraphicsSettings graphics_settings = {};
		uint32_t max_objects = 1000;
		uint32_t max_materials = 100;
	};

	// Common object data unique to each entity (Model matrix, ...)
	struct ObjectData
	{
		glm::vec4 color{ 1.0f };
		glm::mat4 model_matrix{ 1.0f };
	};

	// Global shared scene data
	struct GlobalDescriptorData
	{
		glm::mat4 view_proj;
		glm::mat4 view_proj_light;
		glm::vec4 directional_light_position;
	};

	// Main vulkan state wrapper that manages the lifetime of vulkan resources and execution of renderpasses and renderpipelines
	class Renderer
	{
	public:
		Renderer(RendererInfo &info);
		~Renderer();

		void Init();
		void Shutdown();

		// Starts the current frame and the recording of the command buffer
		void Begin(Scene *scene);

		// Starts mesh render pass
		void MainPassStart(VkCommandBuffer cmd, VkFramebuffer frame_buffer, VkRenderPass render_pass);

		// Ends the main render pass
		void MainPassEnd(VkCommandBuffer cmd);

		// Ends the command buffer recording and submits to queue
		void End();

		// Uploads resources stored in resource manager
		void UploadResources();

		// Gets handle to main renderpass camera
		Camera *GetCamera() { return _camera; }

		// Sets renderer camera
		void SetCamera(Camera *camera) { _camera = camera; }

		// Adds semaphore for graphics queue to wait during submission
		void PushGraphicsSemaphore(VkPipelineStageFlags wait_stage,  VkSemaphore semaphore) {_graphics_wait_stages.push_back(wait_stage), _graphics_wait_semaphores.push_back(semaphore);}

		// Descriptor allocator and layout cache
		std::shared_ptr<vkutil::DescriptorAllocator> _descriptor_allocator;
		std::shared_ptr<vkutil::DescriptorLayoutCache> _descriptor_layout_cache;
	public:
		// ~ OffScreen Rendering

		void InitOffScreenAttachments();

		// Creates an offscreen render_pass;
		void InitOffScreenRenderPass();

		// Creates framebuffer that points the attachments in offscreen render pass to images
		void InitOffScreenFramebuffers();

        // Offscreen renderpass
		VkRenderPass _offscreen_render_pass = {};

        // Offscreen frame buffers
        const VkFormat _offscreen_framebuffer_format = VK_FORMAT_R8G8B8A8_UNORM;
        std::vector<AllocatedImage> _offscreen_images; 
        std::vector<VkImageView> _offscreen_images_views; 
        std::vector<VkFramebuffer> _offscreen_framebuffers;

        // Offscreen depth attachments
		AllocatedImage _offscreen_depth_image = {};
		VkFormat _offscreen_depth_image_format = {};
		VkImageView _offscreen_depth_image_view = {};

		// ~ OffScreen Rendering
	public:
		// Gets handle to the vulkan instance
		const VkInstance GetInstance() const { return _instance; }

		// Gets format of swapchain
		const VkFormat GetSwapchainImageFormat() const {return _swapchain_image_format;}

		// Returns the render pass used by the swapchain images
		const VkRenderPass GetSwapchainRenderPass() const {return _render_pass;}

		// Gets index of current frame
		const uint32_t GetCurrentFrame() const { return _frame_count % frame_overlaps; }

		// Gets index of current frame
		const VkFramebuffer GetCurrentFrameBuffer() const { return _framebuffers[_current_swapchain_image]; }

		// Get shared global descriptor set
		const VkDescriptorSet GetGlobalDescriptorSet() const { return _global_descriptor_set; }

		// Get shared global object descriptor set of current farme
		const VkDescriptorSet GetCurrentGlobalObjectDescriptorSet() const { return _object_descriptor_set[_frame_count % frame_overlaps]; }

		// Get shared global descriptor set layout
		const VkDescriptorSetLayout GetGlobalSetLayout() const { return _global_descriptor_set_layout; }

		// Get shared global object descriptor set layout
		const VkDescriptorSetLayout GetGlobalObjectSetLayout() const { return _object_descriptor_set_layout; }

		// Gets index of current frame data
		const VulkanFrameContext &GetCurrentFrameData() const { return _frame_data[_frame_count % frame_overlaps]; }

		// Returns the vulkan asset manager
		VulkanAssetManager* GetAssetManager() {return _asset_manager.get();}

		// Gets handle to vulkan device struct
		VulkanDevice &GetDevice() { return _device; }

		// Gets handle to vulkan resources deletion queue
		vkutil::DeletionQueue &GetDeletionQueue() { return _deletion_queue; }

		// Gets handle to vulkan resource allocator
		VmaAllocator &GetAllocator() { return _allocator; }

		// Gets handle to vulkan queues struct
		VulkanQueues &GetQueues() { return _queues; }

		// Gets handle to vulkan queues struct
		VulkanQueueFamilyIndices &GetQueueIndices() { return _queue_indices; }

		// Creates and allocates buffer with given size
		AllocatedBuffer CreateBuffer(size_t size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage, VkMemoryPropertyFlags memory_props = 0);

		// Pads size to be compatible with minimum unifrom buffer alignment of physical device
		size_t PadToUniformBufferSize(size_t original_size);

		// Pads size to be compatible with minimum storage buffer alignment of physical device
		size_t PadToStorageBufferSize(size_t original_size);

		// Returns whether or not shader module was created succesefully
		bool LoadShaderModule(const char *path, VkShaderModule *module);

		// Upload mesh to GPU via immediate command buffers
		void UploadMesh(std::shared_ptr<Mesh> mesh);

		// Used for immedaite time and blocking execution of commands
		void ImmediateSubmit(std::function<void(VkCommandBuffer)> &&fn);
	private:
		// Initializes core vulkan structures
		void InitVulkan();

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

		// Creates and allocates descriptors
		void InitGlobalDescriptors();

		// Update Global Descriptor Sets
		void UpdateGlobalDescriptors(Scene *scene, uint32_t frame_index);
	public:
		// General renderer info
		RendererInfo _info = {};

		// Handle to window being rendered to
		const std::shared_ptr<Window> _window;

		// Cameras perspective to render to
		Camera *_camera = nullptr;

		// Global descriptor
		VkDescriptorSet _global_descriptor_set;
		VkDescriptorSetLayout _global_descriptor_set_layout;
		AllocatedBuffer _global_descriptor_buffer;

		// Global Object descriptors (per frame)
		std::vector<VkDescriptorSet> _object_descriptor_set;
		std::vector<AllocatedBuffer> _object_descriptor_buffer;
		VkDescriptorSetLayout _object_descriptor_set_layout;

		VulkanFrameContext _frame_data[frame_overlaps];

		std::unique_ptr<VulkanAssetManager> _asset_manager;
	private:
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

		uint32_t _current_swapchain_image = -1;

		// Main upload struct
		VulkanUploadContext _upload_context = {};

		vkutil::DeletionQueue _deletion_queue;
		uint32_t _frame_count = 0;

		// Main gpu upload resource allocator
		VmaAllocator _allocator;

		// Semaphores the graphics queue will wait for during submission
		std::vector<VkPipelineStageFlags> _graphics_wait_stages = {};
		std::vector<VkSemaphore> _graphics_wait_semaphores = {};
	};
}

#endif