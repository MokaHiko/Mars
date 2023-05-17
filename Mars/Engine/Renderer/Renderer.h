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

#include "Vulkan/VulkanMaterial.h"
#include "Vulkan/VulkanTexture.h"
#include "Vulkan/VulkanMesh.h"

#include "Camera.h"

#include "ECS/Scene.h"

namespace mrs {

	static const uint64_t time_out = 1000000000;
	static const uint32_t frame_overlaps = 2;

	struct GraphicsSettings
	{
		bool vsync = true;
	};

	struct RendererInitInfo
	{
		std::shared_ptr<Window> window;
		std::shared_ptr<Camera> camera;

		GraphicsSettings graphics_settings = {};
	};

	class Renderer
	{
	public:
		const uint32_t max_objects = 100000;
		Renderer(RendererInitInfo& info);
		~Renderer();

		virtual void Init();
		virtual void Shutdown();

		virtual void Begin(Scene* scene);
		virtual void End();

		void UploadResources();

		// Getters
		inline VkInstance GetInstance() { return _instance; }
		inline VkRenderPass GetRenderPass() { return _render_pass; }
		inline VulkanDevice& GetDevice() { return _device; }
		inline vkutil::DeletionQueue& GetDeletionQueue() { return _deletion_queue; }
		inline VulkanQueues& GetQueues() { return _queues; }

	public:
		// DEMO: SHADOW MAPS
		void CreateOffScreenFramebuffer();
		void InitOffScreenPipeline();

		void DrawShadowMap(VkCommandBuffer cmd, Scene* scene);

		VkFramebuffer _offscreen_framebuffer;
		AllocatedImage _offscreen_depth_image;
		VkImageView _offscreen_depth_image_view;

		VkRenderPass _offscreen_render_pass;
		VkPipeline _offscreen_render_pipeline;

		VkSampler _shadow_map_sampler;
		VkDescriptorSetLayout _shadow_map_descriptor_layout;
		VkDescriptorSet _shadow_map_descriptor;
	public:
		// Handle to window being rendered to
		const std::shared_ptr<Window> _window;

		// Cameras perspective to render to
		std::shared_ptr<Camera> _camera;

		struct ObjectData
		{
			glm::vec4 color;
			glm::mat4 model_matrix;
		};

		struct GlobalDescriptorData {
			glm::mat4 view_proj;
			glm::mat4 view_proj_light;
			glm::vec4 directional_light_position;
		};

		// Global descriptor
		VkDescriptorSet global_descriptor_set;
		VkDescriptorSetLayout global_descriptor_set_layout;
		AllocatedBuffer global_descriptor_buffer;

		// Object descriptors (per frame)
		std::vector<VkDescriptorSet> object_descriptor_set;
		std::vector<AllocatedBuffer> object_descriptor_buffer;
		VkDescriptorSetLayout object_descriptor_set_layout;

		VkSampler _default_image_sampler;
		VkDescriptorSetLayout _default_image_set_layout;
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

		// Creates and allocates descriptors 
		void InitDescriptors();
	public:
		// Creates graphics pipelines based of parameters
		void InitPipelines();
	private:
		inline uint32_t GetCurrentFrame() const { return _frame_count % frame_overlaps; }

		// Pads size to be compatible with minimum unifrom buffer alignment of physical device
		size_t PadToUniformBufferSize(size_t original_size);

		// Upload texture to GPU via immediate command buffers
		void UploadTexture(std::shared_ptr<Texture> texture);

		// Upload mesh to GPU via immediate command buffers
		void UploadMesh(std::shared_ptr<Mesh> mesh);

	private:
		// Creates indirect command buffers
		void InitIndirectCommands();

		void DrawObjects(VkCommandBuffer cmd, Scene* scene);
		
		// ~ INDIRECT DRAWING
		struct IndirectBatch 
		{
			Mesh* mesh;
			Material* material;

			uint32_t first; // batches first instance in draw indirect buffer
			uint32_t count; // batch member count
		};

		// Returns vector if indirect batches from renderables from scene
		std::vector<IndirectBatch> GetRenderablesAsBatches(Scene* scene);

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
		VulkanFrameContext _frame_data[frame_overlaps];
		uint32_t _current_swapchain_image = -1;

		// Main upload struct
		VulkanUploadContext _upload_context = {};

		vkutil::DeletionQueue _deletion_queue;
		uint32_t _frame_count = 0;

	private:
		// ~ Vulkan resource management

		// Main gpu upload resource allocator
		VmaAllocator _allocator;

		// Desccriptor allocator and layout cache
		std::shared_ptr<vkutil::DescriptorAllocator> _descriptor_allocator;
		std::shared_ptr<vkutil::DescriptorLayoutCache> _descriptor_layout_cache;
	};
}

#endif