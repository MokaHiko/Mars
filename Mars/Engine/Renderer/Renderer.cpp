#include "Renderer.h"

#include <SDL2/SDL.h>
#include <VkBootstrap.h>

#include "Vulkan/VulkanUtils.h"
#include "Vulkan/VulkanInitializers.h"

namespace mrs
{
	Renderer::Renderer(const std::shared_ptr<Window> window)
		:_window(window) {}

	Renderer::~Renderer() {}

	void Renderer::Init()
	{
		// CORE VULKAN STRUCTURES

		// Instance
		vkb::InstanceBuilder builder;
		auto intance_result = builder.set_app_name("First App")
			.request_validation_layers(true)
			.require_api_version(1, 2, 0)
			.use_default_debug_messenger()
			.build();

		vkb::Instance vkb_instance = intance_result.value();
		_instance = vkb_instance.instance;
		_debug_messenger = vkb_instance.debug_messenger;

		if (SDL_Vulkan_CreateSurface((SDL_Window*)_window->GetNativeWindow(), _instance, &_surface) != SDL_TRUE) {
			printf("%s", SDL_GetError());
		}

		// Select physical device 
		vkb::PhysicalDeviceSelector device_selector(vkb_instance);
		auto physical_device_result = device_selector
			.set_minimum_version(1, 1)
			.set_surface(_surface)
			.select()
			.value();
		_device.physical_device = physical_device_result.physical_device;


		// Create device and get queue and queue family indices 
		vkb::DeviceBuilder device_builder{ physical_device_result };
		vkb::Device vkb_device = device_builder.build().value();
		_device.device = vkb_device.device;

		_queues.graphics = vkb_device.get_queue(vkb::QueueType::graphics).value();
		_queue_indices.graphics = vkb_device.get_queue_index(vkb::QueueType::graphics).value();

		_queues.present = vkb_device.get_queue(vkb::QueueType::present).value();
		_queue_indices.present = vkb_device.get_queue_index(vkb::QueueType::present).value();

		_queues.transfer = vkb_device.get_queue(vkb::QueueType::transfer).value();
		_queue_indices.transfer = vkb_device.get_queue_index(vkb::QueueType::transfer).value();

		_queues.compute = vkb_device.get_queue(vkb::QueueType::compute).value();
		_queue_indices.compute = vkb_device.get_queue_index(vkb::QueueType::compute).value();

		_deletion_queue.Push([=]() {
			vkDestroyDevice(_device.device, nullptr);
			vkb::destroy_debug_utils_messenger(_instance, _debug_messenger, nullptr);
			vkDestroySurfaceKHR(_instance, _surface, nullptr);
			vkDestroyInstance(_instance, nullptr);
			});

		InitSwapchain();
		InitDefaultRenderPass();
		InitFramebuffers();
		InitCommands();
		InitSyncStructures();
	}

	void Renderer::Shutdown()
	{
		vkDeviceWaitIdle(_device.device);
		_deletion_queue.Flush();
	}

	void Renderer::InitSwapchain()
	{
		vkb::SwapchainBuilder builder{ _device.physical_device, _device.device, _surface };
		vkb::Swapchain vkb_swapchain = builder.use_default_format_selection()
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(_window->GetWidth(), _window->GetHeight())
			.build()
			.value();

		_swapchain = vkb_swapchain.swapchain;
		_swapchain_image_format = vkb_swapchain.image_format;
		_swapchain_images = vkb_swapchain.get_images().value();

		// Swap chain image views are created
		_swapchain_image_views = vkb_swapchain.get_image_views().value();

		_deletion_queue.Push([=]() {
			for (auto image_view : _swapchain_image_views) {
				vkDestroyImageView(_device.device, image_view, nullptr);
			}
			});
	}

	void Renderer::InitCommands()
	{
		VkCommandPoolCreateInfo graphics_cp_info = vkinit::command_pool_create_info(_queue_indices.graphics, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		VkCommandPoolCreateInfo upload_cp_info = vkinit::command_pool_create_info(_queue_indices.graphics, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		// Create a command pool & buffer for each frame to send render commands async
		for (uint32_t i = 0; i < frame_overlaps; i++) {
			vkCreateCommandPool(_device.device, &graphics_cp_info, nullptr, &_frame_data[i].command_pool);

			VkCommandBufferAllocateInfo alloc_info = vkinit::command_buffer_alloc_info(_frame_data[i].command_pool);
			vkAllocateCommandBuffers(_device.device, &alloc_info, &_frame_data[i].command_buffer);

			_deletion_queue.Push([=]() {
				vkDestroyCommandPool(_device.device, _frame_data[i].command_pool, nullptr);
				});
		}

		// Create one commmand pool & buffer for all upload/transfer operations
		vkCreateCommandPool(_device.device, &graphics_cp_info, nullptr, &_upload_context.command_pool);
		VkCommandBufferAllocateInfo alloc_info = vkinit::command_buffer_alloc_info(_upload_context.command_pool);
		vkAllocateCommandBuffers(_device.device, &alloc_info, &_upload_context.command_buffer);

		_deletion_queue.Push([=]() {
			vkDestroyCommandPool(_device.device, _upload_context.command_pool, nullptr);
			});
	}

	void Renderer::InitDefaultRenderPass()
	{
		// Create render pass attachments and references
		VkAttachmentDescription color_attachment = {};
		color_attachment.format = _swapchain_image_format;
		color_attachment.flags = 0;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		std::vector<VkAttachmentDescription> attachments = { color_attachment };

		VkAttachmentReference color_attachment_reference = {};
		color_attachment_reference.attachment = 0;
		color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// Create subpasses
		VkSubpassDescription main_subpass = {};
		main_subpass.colorAttachmentCount = 1;
		main_subpass.pColorAttachments = &color_attachment_reference;
		main_subpass.pDepthStencilAttachment = nullptr;
		main_subpass.inputAttachmentCount = 0;
		main_subpass.pInputAttachments = 0;

		// Create dependencies
		VkSubpassDependency color_dependency = {};
		// ~ after subpass external - outputs to color attachment - using color_attachment write memory access
		color_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		color_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		color_dependency.srcAccessMask = 0;

		// ~ bofere subpass external - outputs to color attachment - using color_attachment write memory access
		color_dependency.dstSubpass = 0;
		color_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		color_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.pNext = nullptr;

		render_pass_info.attachmentCount = 1;
		render_pass_info.pAttachments = attachments.data();

		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &main_subpass;

		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &color_dependency;

		VK_CHECK(vkCreateRenderPass(_device.device, &render_pass_info, nullptr, &_render_pass));

		_deletion_queue.Push([=]() {
			vkDestroyRenderPass(_device.device, _render_pass, nullptr);
			});
	}

	void Renderer::InitFramebuffers()
	{
		_framebuffers.resize(_swapchain_images.size());

		// Link render pass attachments to swapchain images by frame buffers
		for (uint32_t i = 0; i < _swapchain_images.size(); i++) {
			VkFramebufferCreateInfo frame_buffer_info = {};
			frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frame_buffer_info.pNext = nullptr;

			frame_buffer_info.attachmentCount = 1;
			frame_buffer_info.pAttachments = &_swapchain_image_views[i];

			// 2d attachments buffer
			frame_buffer_info.width = _window->GetWidth();
			frame_buffer_info.height = _window->GetHeight();
			frame_buffer_info.layers = 1;

			frame_buffer_info.renderPass = _render_pass;
			VK_CHECK(vkCreateFramebuffer(_device.device, &frame_buffer_info, nullptr, &_framebuffers[i]));

			_deletion_queue.Push([=]() {
				vkDestroyFramebuffer(_device.device, _framebuffers[i], nullptr);
				});
		}
	}

	void Renderer::InitSyncStructures()
	{
		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.pNext = nullptr;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkSemaphoreCreateInfo semaphore_info = {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphore_info.pNext = nullptr;
		semaphore_info.flags = 0;

		// Create sync structures for frames
		for (auto& frame : _frame_data) {
			vkCreateFence(_device.device, &fence_info, nullptr, &frame.render_fence);

			vkCreateSemaphore(_device.device, &semaphore_info, nullptr, &frame.present_semaphore);
			vkCreateSemaphore(_device.device, &semaphore_info, nullptr, &frame.render_semaphore);

			_deletion_queue.Push([=]() {
				vkDestroyFence(_device.device, frame.render_fence, nullptr);

				vkDestroySemaphore(_device.device, frame.present_semaphore, nullptr);
				vkDestroySemaphore(_device.device, frame.render_semaphore, nullptr);
				});
		}

		// Create sync structures for upload
		vkCreateFence(_device.device, &fence_info, nullptr, &_upload_context.upload_fence);
		_deletion_queue.Push([=]() {
			vkDestroyFence(_device.device, _upload_context.upload_fence, nullptr);
			});
	}
	void Renderer::Update()
	{
		// Get current frame index, current frame data, current cmd bufffer
		uint32_t n_frame = GetCurrentFrame();
		auto& frame = _frame_data[GetCurrentFrame()];
		VkCommandBuffer cmd = frame.command_buffer;

		// Wait till render fence has been flagged
		static const uint64_t time_out = 1000000000;
		VK_CHECK(vkWaitForFences(_device.device, 1, &frame.render_fence, VK_TRUE, time_out));
		vkResetFences(_device.device, 1, &frame.render_fence);

		// Get current image to render to 
		// Make sure image has been acquired before submitting
		uint32_t current_image = -1;
		vkAcquireNextImageKHR(_device.device, _swapchain, time_out, frame.present_semaphore, VK_NULL_HANDLE, &current_image);

		// ~ Begin Recording 
		VK_CHECK(vkResetCommandBuffer(cmd, 0));
		VkCommandBufferBeginInfo cmd_begin_info = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begin_info));

		VkRect2D area = {};
		area.extent = { _window->GetWidth(), _window->GetHeight() };
		area.offset = { 0,0 };

		VkClearValue clear_value = {};
		clear_value.color = { 0.1f, (float)sin(_frame_count / 120.0f), 0.1f };

		VkRenderPassBeginInfo render_pass_begin_info = vkinit::render_pass_begin_info(_framebuffers[current_image], _render_pass, area, &clear_value, 1);
		vkCmdBeginRenderPass(cmd, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		// General render commands
		{

		}

		vkCmdEndRenderPass(cmd);
		VK_CHECK(vkEndCommandBuffer(cmd));

		// Submit commands
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = nullptr;

		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cmd;

		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &frame.present_semaphore;

		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &frame.render_semaphore;

		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit_info.pWaitDstStageMask = &wait_stage;

		VK_CHECK(vkQueueSubmit(_queues.graphics, 1, &submit_info, frame.render_fence));

		// Present image
		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext = nullptr;

		present_info.pImageIndices = &current_image;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &_swapchain;

		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores= &frame.render_semaphore;

		VK_CHECK(vkQueuePresentKHR(_queues.graphics, &present_info));

		// Update frame counter
		++_frame_count;
	}
}