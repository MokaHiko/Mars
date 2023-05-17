#include "Renderer.h"

#include <SDL2/SDL.h>
#include <VkBootstrap.h>

#include "Util/Utils.h"
#include "Vulkan/VulkanUtils.h"
#include "Vulkan/VulkanInitializers.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

// TODO: Remove
#include <glm/gtc/matrix_transform.hpp>
#include <ImGui/MarsImGui.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>

namespace mrs
{
	Renderer::Renderer(RendererInitInfo& info)
		:_window(info.window), _camera(info.camera) {}

	Renderer::~Renderer() {}

	void Renderer::Init()
	{
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
		VkPhysicalDeviceFeatures required_features = {};
		required_features.multiDrawIndirect = VK_TRUE;
		auto physical_device_result = device_selector
			.set_minimum_version(1, 1)
			.set_required_features(required_features)
			.set_surface(_surface)
			.select()
			.value();
		_device.physical_device = physical_device_result.physical_device;
		_physical_device_props = physical_device_result.properties;

		// Create device and get queue and queue family indices 
		vkb::DeviceBuilder device_builder{ physical_device_result };

		VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features = {};
		shader_draw_parameters_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
		shader_draw_parameters_features.pNext = nullptr;
		shader_draw_parameters_features.shaderDrawParameters = VK_TRUE;

		vkb::Device vkb_device = device_builder.add_pNext(&shader_draw_parameters_features).build().value();
		auto yes = vkb_device.physical_device.features.multiDrawIndirect;

		_device.device = vkb_device.device;

		_queues.graphics = vkb_device.get_queue(vkb::QueueType::graphics).value();
		_queue_indices.graphics = vkb_device.get_queue_index(vkb::QueueType::graphics).value();

		_queues.present = vkb_device.get_queue(vkb::QueueType::present).value();
		_queue_indices.present = vkb_device.get_queue_index(vkb::QueueType::present).value();

		_queues.transfer = vkb_device.get_queue(vkb::QueueType::transfer).value();
		_queue_indices.transfer = vkb_device.get_queue_index(vkb::QueueType::transfer).value();

		_queues.compute = vkb_device.get_queue(vkb::QueueType::compute).value();
		_queue_indices.compute = vkb_device.get_queue_index(vkb::QueueType::compute).value();

		VmaAllocatorCreateInfo allocator_info = {};
		allocator_info.physicalDevice = _device.physical_device;
		allocator_info.device = _device.device;
		allocator_info.instance = _instance;
		vmaCreateAllocator(&allocator_info, &_allocator);

		_deletion_queue.Push([=]() {
			vmaDestroyAllocator(_allocator);

			vkDestroyDevice(_device.device, nullptr);
			vkb::destroy_debug_utils_messenger(_instance, _debug_messenger, nullptr);

			vkDestroySurfaceKHR(_instance, _surface, nullptr);
			vkDestroyInstance(_instance, nullptr);
			});

		InitSwapchain();
		InitDefaultRenderPass();
		InitFramebuffers();

		InitCommands();

		// DEMO: 
		InitIndirectCommands();
		CreateOffScreenFramebuffer();

		InitSyncStructures();
		InitDescriptors();

		// TODO: Make optional 
		// Ready Engine for ImGUi
		MarsImGuiInit(this);
	}

	void Renderer::Shutdown()
	{
		vkDeviceWaitIdle(_device.device);

		// Shutdown descriptors managers
		_descriptor_layout_cache->Clear();
		_descriptor_allocator->CleanUp();

		_deletion_queue.Flush();
	}

	void Renderer::InitIndirectCommands()
	{
		size_t max_indirect_commands = 1000;

		// Create indirect buffer per frame
		for (int i = 0; i < frame_overlaps; i++)
		{
			_frame_data[i].indirect_buffer = CreateBuffer(max_indirect_commands * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 0);

			_deletion_queue.Push([=]() {
				vmaDestroyBuffer(_allocator, _frame_data[i].indirect_buffer.buffer, _frame_data[i].indirect_buffer.allocation);
				});
		}
	}

	void Renderer::DrawObjects(VkCommandBuffer cmd, Scene* scene)
	{
		uint32_t n_frame = GetCurrentFrame();

		// Update global descriptor
		_camera->UpdateViewProj();
		void* global_data;
		vmaMapMemory(_allocator, global_descriptor_buffer.allocation, &global_data);

		// Camera
		GlobalDescriptorData global_info = {};
		global_info.view_proj = _camera->GetViewProj();

		// Lights
		auto lights_view = scene->Registry()->view<Transform, DirectionalLight>();
		for (auto entity : lights_view) {
			Transform& transform = Entity(entity, scene).GetComponent<Transform>();
			global_info.directional_light_position = glm::vec4(transform.position, 0.0f);
			global_info.view_proj_light = _camera->GetProj() * glm::translate(glm::mat4(1.0f), transform.position);
			break;
		}

		// Renderable Objects
		memcpy(global_data, &global_info, sizeof(GlobalDescriptorData));
		vmaUnmapMemory(_allocator, global_descriptor_buffer.allocation);

		// Bind object data storage buffer
		void* objectData;
		vmaMapMemory(_allocator, object_descriptor_buffer[n_frame].allocation, &objectData);
		ObjectData* s_data = (ObjectData*)(objectData);
		ObjectData obj_info = {};

		auto view = scene->Registry()->view<Transform, RenderableObject>();

		uint32_t ctr = 0;
		for (auto entity : view) {
			auto& game_object = Entity(entity, scene);
			Transform& transform = game_object.GetComponent<Transform>();

			glm::mat4 model(1.0f);
			glm::vec3 pos = transform.position;

			model = glm::translate(model, pos);
			model = glm::rotate(model, (float)glm::radians(glm::sin(SDL_GetTicks() * 0.0001) * 360), glm::vec3(0.4f, 0.2f, 0.4f));
			model = glm::scale(model, transform.scale);

			obj_info.model_matrix = model;
			s_data[ctr] = obj_info;

			ctr++;
		}
		vmaUnmapMemory(_allocator, object_descriptor_buffer[n_frame].allocation);

		// Bind global and object descriptors
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _default_pipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _default_pipeline_layout, 0, 1, &global_descriptor_set, 0, nullptr);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _default_pipeline_layout, 1, 1, &object_descriptor_set[n_frame], 0, nullptr);

		// Draw Indirect
		static bool recorded = false;
		if (!recorded)
		{
			for (int i = 0; i < frame_overlaps; i++) {
				VkDrawIndexedIndirectCommand* draw_commands;
				vmaMapMemory(_allocator, _frame_data[i].indirect_buffer.allocation, (void**)&draw_commands);

				// Encode draw commands for each renderable ahead of time
				ctr = 0;
				for (auto entity : view) {
					auto& renderable = Entity(entity, scene).GetComponent<RenderableObject>();
					draw_commands[ctr].vertexOffset = 0;
					draw_commands[ctr].indexCount = renderable.mesh->_index_count;
					draw_commands[ctr].instanceCount = 1;
					draw_commands[ctr].firstInstance = ctr;
					ctr++;
				}
				vmaUnmapMemory(_allocator, _frame_data[i].indirect_buffer.allocation);
			}
			recorded = true;
		}

		// Sort rederables into batches
		std::vector<IndirectBatch> batches = GetRenderablesAsBatches(scene);

		for (auto& batch : batches) {
			// Bind batch vertex and index buffer
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(cmd, 0, 1, &batch.mesh->_buffer.buffer, &offset);

			if (batch.mesh->_index_count > 0) {
				vkCmdBindIndexBuffer(cmd, batch.mesh->_index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
			}

			// Bind batch material
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _default_pipeline_layout, 2, 1, &batch.material->texture_set, 0, nullptr);

			// DEMO: If Model Has shadows
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _default_pipeline_layout, 3, 1, &_shadow_map_descriptor, 0, nullptr);

			// Draw batch
			VkDeviceSize batch_stride = sizeof(VkDrawIndexedIndirectCommand);
			uint32_t indirect_offset = batch.first * batch_stride;

			vkCmdDrawIndexedIndirect(cmd, _frame_data[n_frame].indirect_buffer.buffer, indirect_offset, batch.count, batch_stride);
		}

		// [Profiling]
		ImGui::Begin("Renderer Stats");
		ImGui::Text("Objects: %d", ctr);
		ImGui::Text("Draw Calls: %d", batches.size());
		ImGui::End();
	}

	std::vector<Renderer::IndirectBatch> Renderer::GetRenderablesAsBatches(Scene* scene)
	{
		std::vector<IndirectBatch> batches;

		auto renderables = scene->Registry()->view<RenderableObject>();

		Material* last_material = nullptr;
		Mesh* last_mesh = nullptr;

		for (auto entity : renderables)
		{
			Entity e(entity, scene);
			auto& renderable = e.GetComponent<RenderableObject>();

			// Check if new batch
			bool new_batch = renderable.material.get() != last_material || renderable.mesh.get() != last_mesh;
			last_mesh = renderable.mesh.get();
			last_material = renderable.material.get();

			if (new_batch) {
				IndirectBatch batch = {};
				batch.first = 0;
				batch.count = 1;
				batch.material = renderable.material.get();
				batch.mesh = renderable.mesh.get();

				batches.push_back(batch);
			}
			else {
				batches.back().count++;
			}
		}

		return batches;
	}

	void Renderer::UploadTexture(std::shared_ptr<Texture> texture)
	{
		void* pixel_ptr = nullptr;

		// Copy to staging buffer
		AllocatedBuffer staging_buffer = CreateBuffer(texture->pixel_data.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		vmaMapMemory(_allocator, staging_buffer.allocation, &pixel_ptr);
		memcpy(pixel_ptr, texture->pixel_data.data(), texture->pixel_data.size());
		vmaUnmapMemory(_allocator, staging_buffer.allocation);

		// Free pixel data
		texture->pixel_data.clear();
		texture->pixel_data.shrink_to_fit();

		// Create texture 
		VkExtent3D extent;
		extent.width = texture->_width;
		extent.height = texture->_height;
		extent.depth = 1;

		VkImageCreateInfo image_create_info = vkinit::image_create_info(texture->_format, extent, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

		VmaAllocationCreateInfo vma_alloc_info = {};
		vma_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateImage(_allocator, &image_create_info, &vma_alloc_info, &texture->_image.image, &texture->_image.allocation, nullptr);

		// Copy data to texture via immediate mode submit
		ImmediateSubmit([&](VkCommandBuffer cmd) {

			// ~ Transition to transfer optimal
			VkImageSubresourceRange range = {};
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.layerCount = 1;
			range.baseArrayLayer = 0;
			range.levelCount = 1;
			range.baseMipLevel = 0;

			VkImageMemoryBarrier image_to_transfer_barrier = {};
			image_to_transfer_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			image_to_transfer_barrier.pNext = nullptr;

			image_to_transfer_barrier.image = texture->_image.image;
			image_to_transfer_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			image_to_transfer_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

			image_to_transfer_barrier.srcAccessMask = 0;
			image_to_transfer_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			image_to_transfer_barrier.subresourceRange = range;

			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_to_transfer_barrier);

			// ~ Copy to from staging to texture
			VkBufferImageCopy region = {};
			region.bufferImageHeight = 0;
			region.bufferRowLength = 0;
			region.bufferOffset = 0;

			region.imageExtent = extent;
			region.imageOffset = { 0 };
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageSubresource.mipLevel = 0;

			vkCmdCopyBufferToImage(cmd, staging_buffer.buffer, texture->_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			// ~ Transition to from shader read only 
			VkImageMemoryBarrier image_to_shader_barrier = image_to_transfer_barrier;
			image_to_shader_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			image_to_shader_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			image_to_shader_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			image_to_shader_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_to_shader_barrier);
			});

		// Create iamge view
		VkImageViewCreateInfo image_view_info = vkinit::image_view_create_info(texture->_image.image, texture->_format, VK_IMAGE_ASPECT_COLOR_BIT);
		VK_CHECK(vkCreateImageView(_device.device, &image_view_info, nullptr, &texture->_image_view));

		// Clean up
		vmaDestroyBuffer(_allocator, staging_buffer.buffer, staging_buffer.allocation);
		_deletion_queue.Push([=]() {
			vkDestroyImageView(_device.device, texture->_image_view, nullptr);
			vmaDestroyImage(_allocator, texture->_image.image, texture->_image.allocation);
			});
	}

	void Renderer::UploadMesh(std::shared_ptr<Mesh> mesh)
	{
		// Create and upload data to staging bufffer
		const size_t buffer_size = mesh->_vertices.size() * sizeof(Vertex);
		AllocatedBuffer staging_buffer = CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

		void* data;
		vmaMapMemory(_allocator, staging_buffer.allocation, &data);
		memcpy(data, mesh->_vertices.data(), buffer_size);
		vmaUnmapMemory(_allocator, staging_buffer.allocation);

		// Create and transfer data to vertex bufffer stored in vulkan mesh 
		mesh->_buffer = CreateBuffer(buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 0);

		// Copy data to vertex buffer via immediate commands
		ImmediateSubmit([=](VkCommandBuffer cmd) {
			VkBufferCopy region = {};
			region.dstOffset = 0;
			region.size = buffer_size;
			region.srcOffset = 0;
			vkCmdCopyBuffer(cmd, staging_buffer.buffer, mesh->_buffer.buffer, 1, &region);
			});

		// Create index buffer if indices available
		if (mesh->_index_count > 0) {
			const size_t index_buffer_size = mesh->_index_count * sizeof(uint32_t);
			AllocatedBuffer index_staging_buffer = CreateBuffer(index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

			void* index_data;
			vmaMapMemory(_allocator, index_staging_buffer.allocation, &index_data);
			memcpy(index_data, mesh->_indices.data(), index_buffer_size);
			vmaUnmapMemory(_allocator, index_staging_buffer.allocation);

			mesh->_index_buffer = CreateBuffer(index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 0);

			ImmediateSubmit([=](VkCommandBuffer cmd) {
				VkBufferCopy region = {};
				region.dstOffset = 0;
				region.size = index_buffer_size;
				region.srcOffset = 0;
				vkCmdCopyBuffer(cmd, index_staging_buffer.buffer, mesh->_index_buffer.buffer, 1, &region);
				});

			vmaDestroyBuffer(_allocator, index_staging_buffer.buffer, index_staging_buffer.allocation);
		}

		// Clean up staging buffer and queue vertex buffer for deletion
		mesh->_vertices.clear();
		mesh->_vertices.shrink_to_fit();
		mesh->_indices.clear();
		mesh->_indices.shrink_to_fit();

		vmaDestroyBuffer(_allocator, staging_buffer.buffer, staging_buffer.allocation);

		_deletion_queue.Push([=]() {
			vmaDestroyBuffer(_allocator, mesh->_buffer.buffer, mesh->_buffer.allocation);

			if (mesh->_index_count > 0) {
				vmaDestroyBuffer(_allocator, mesh->_index_buffer.buffer, mesh->_index_buffer.allocation);
			}
			});
	}

	void Renderer::UploadResources()
	{
		// Upload each resource type

		for (auto& it : ResourceManager::Get()._meshes) {
			UploadMesh(it.second);
		}

		for (auto& it : ResourceManager::Get()._textures) {
			UploadTexture(it.second);
		}

		// ~ Materials
		{
			// Create Texture Sampler 
			VkSamplerCreateInfo default_sampler_info = vkinit::sampler_create_info(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
			VK_CHECK(vkCreateSampler(_device.device, &default_sampler_info, nullptr, &_default_image_sampler));

			for (auto& it : ResourceManager::Get()._materials) {

				VkDescriptorImageInfo image_buffer_info = {};
				image_buffer_info.sampler = _default_image_sampler;
				image_buffer_info.imageView = Texture::Get("default_texture")->_image_view;
				image_buffer_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				vkutil::DescriptorBuilder::Begin(_descriptor_layout_cache.get(), _descriptor_allocator.get())
					.BindImage(0, &image_buffer_info, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
					.Build(&it.second->texture_set, &_default_image_set_layout);
			}

			// DEMO: Create shadow descriptor and layout
			VkDescriptorImageInfo shadow_map_image_info = {};
			shadow_map_image_info.sampler = _shadow_map_sampler;
			shadow_map_image_info.imageView = _offscreen_depth_image_view;
			shadow_map_image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

			vkutil::DescriptorBuilder::Begin(_descriptor_layout_cache.get(), _descriptor_allocator.get())
				.BindImage(0, &shadow_map_image_info, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.Build(&_shadow_map_descriptor, &_shadow_map_descriptor_layout);

			_deletion_queue.Push([=]() {
				vkDestroySampler(_device.device, _default_image_sampler, nullptr);
				});
		}
	}

	void Renderer::CreateOffScreenFramebuffer()
	{
		// Create frame buffer attachment
		VkExtent3D extent = {};
		extent.depth = 1;
		extent.width = _window->GetWidth();
		extent.height = _window->GetHeight();

		VkFormat depth_format = VK_FORMAT_D32_SFLOAT;
		VkImageCreateInfo depth_image_info = vkinit::image_create_info(depth_format, extent, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		VmaAllocationCreateInfo vmaaloc_info = {};
		vmaaloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		vmaaloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VK_CHECK(vmaCreateImage(_allocator, &depth_image_info, &vmaaloc_info, &_offscreen_depth_image.image, &_offscreen_depth_image.allocation, nullptr));

		// Create offscreen attachment view
		VkImageViewCreateInfo depth_image_view_info = vkinit::image_view_create_info(_offscreen_depth_image.image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
		VK_CHECK(vkCreateImageView(_device.device, &depth_image_view_info, nullptr, &_offscreen_depth_image_view));

		// Create render pass
		VkAttachmentDescription depth_attachment = {};
		depth_attachment.format = VK_FORMAT_D32_SFLOAT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkAttachmentReference depth_attachment_reference = {};
		depth_attachment_reference.attachment = 0;
		depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pDepthStencilAttachment = &depth_attachment_reference;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;

		std::array<VkSubpassDependency, 2> dependencies = {};

		// Between external subpass and current subpass
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;	// reading of last frame
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL; // our color render pass
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT; // after depth testing has finished 
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // before shader reads data
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo _offscreen_render_pass_info = {};
		_offscreen_render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

		_offscreen_render_pass_info.attachmentCount = 1;
		_offscreen_render_pass_info.pAttachments = &depth_attachment;

		// subpasses
		_offscreen_render_pass_info.subpassCount = 1;
		_offscreen_render_pass_info.pSubpasses = &subpass;

		// dependencies between subpasses
		_offscreen_render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
		_offscreen_render_pass_info.pDependencies = dependencies.data();

		VK_CHECK(vkCreateRenderPass(_device.device, &_offscreen_render_pass_info, nullptr, &_offscreen_render_pass));

		// Create Frame buffer
		VkFramebufferCreateInfo offscreen_framebuffer_info = {};
		offscreen_framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

		offscreen_framebuffer_info.renderPass = _offscreen_render_pass;

		offscreen_framebuffer_info.attachmentCount = 1;
		offscreen_framebuffer_info.width = _window->GetWidth();
		offscreen_framebuffer_info.height = _window->GetHeight();
		offscreen_framebuffer_info.pAttachments = &_offscreen_depth_image_view;
		offscreen_framebuffer_info.layers = 1;

		VK_CHECK(vkCreateFramebuffer(_device.device, &offscreen_framebuffer_info, nullptr, &_offscreen_framebuffer));

		// DEMO: Create Image Sampelr
		VkSamplerCreateInfo shadow_map_sampler_info = vkinit::sampler_create_info(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		shadow_map_sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
		shadow_map_sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		shadow_map_sampler_info.mipLodBias = 0.0f;
		shadow_map_sampler_info.maxAnisotropy = 1.0f;
		shadow_map_sampler_info.minLod = 0.0f;
		shadow_map_sampler_info.maxLod = 1.0f;

		VK_CHECK(vkCreateSampler(_device.device, &shadow_map_sampler_info, nullptr, &_shadow_map_sampler));

		_deletion_queue.Push([=]() {
			vkDestroySampler(_device.device, _shadow_map_sampler, nullptr);
			vkDestroyFramebuffer(_device.device, _offscreen_framebuffer, nullptr);
			vkDestroyRenderPass(_device.device, _offscreen_render_pass, nullptr);
			vkDestroyImageView(_device.device, _offscreen_depth_image_view, nullptr);
			vmaDestroyImage(_allocator, _offscreen_depth_image.image, _offscreen_depth_image.allocation);
			});
	}

	void Renderer::InitOffScreenPipeline()
	{
		vkutil::PipelineBuilder pipeline_builder = {};

		// Pipeline view port
		pipeline_builder._scissor.extent = { _window->GetWidth(), _window->GetHeight() };
		pipeline_builder._scissor.offset = { 0, 0 };

		pipeline_builder._viewport.x = 0.0f;
		pipeline_builder._viewport.y = 0.0f;
		pipeline_builder._viewport.width = (float)(_window->GetWidth());
		pipeline_builder._viewport.height = (float)(_window->GetHeight());
		pipeline_builder._viewport.minDepth = 0.0f;
		pipeline_builder._viewport.maxDepth = 1.0f;

		// Shaders modules
		bool loaded = false;
		VkShaderModule vertex_shader_module;
		loaded = LoadShaderModule("Assets/Shaders/offscreen_shader.vert.spv", &vertex_shader_module);
		pipeline_builder._shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader_module));

		// Vertex input (Primitives and Vertex Input Descriptions
		pipeline_builder._input_assembly = vkinit::pipeline_input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		VertexInputDescription& vb_desc = Vertex::GetDescription();

		pipeline_builder._vertex_input_info = vkinit::pipeline_vertex_input_state_create_info();

		pipeline_builder._vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vb_desc.bindings.size());
		pipeline_builder._vertex_input_info.pVertexBindingDescriptions = vb_desc.bindings.data();

		pipeline_builder._vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vb_desc.attributes.size());
		pipeline_builder._vertex_input_info.pVertexAttributeDescriptions = vb_desc.attributes.data();

		// Graphics Settings

		// Disable cull so all faces contribute to shadows
		pipeline_builder._rasterizer = vkinit::pipeline_rasterization_state_create_info(VK_POLYGON_MODE_FILL);
		pipeline_builder._rasterizer.cullMode = VK_CULL_MODE_NONE;

		pipeline_builder._multisampling = vkinit::pipeline_mulitisample_state_create_info();
		pipeline_builder._color_blend_attachment = {};
		pipeline_builder._depth_stencil = vkinit::pipeline_depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

		// Offscreen specific

		// Create pipeline layout
		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();

		std::vector<VkDescriptorSetLayout> descriptor_layouts = { global_descriptor_set_layout , object_descriptor_set_layout, _default_image_set_layout };
		pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_layouts.size());
		pipeline_layout_info.pSetLayouts = descriptor_layouts.data();

		pipeline_builder._pipeline_layout = _default_pipeline_layout;

		_offscreen_render_pipeline = pipeline_builder.Build(_device.device, _offscreen_render_pass, true);

		if (_offscreen_render_pipeline == VK_NULL_HANDLE) {
			printf("Failed to create pipeline!");
		}

		// Clean Up
		vkDestroyShaderModule(_device.device, vertex_shader_module, nullptr);
		_deletion_queue.Push([=]() {
			vkDestroyPipeline(_device.device, _offscreen_render_pipeline, nullptr);
			});
	}

	void Renderer::DrawShadowMap(VkCommandBuffer cmd, Scene* scene)
	{
		uint32_t n_frame = GetCurrentFrame();

		// Update global descriptor
		_camera->UpdateViewProj();
		void* global_data;
		vmaMapMemory(_allocator, global_descriptor_buffer.allocation, &global_data);

		// Update Camera
		GlobalDescriptorData global_info = {};
		global_info.view_proj = _camera->GetViewProj();

		// Set up Lights
		auto lights_view = scene->Registry()->view<Transform, DirectionalLight>();
		for (auto entity : lights_view) {
			Transform& transform = Entity(entity, scene).GetComponent<Transform>();
			global_info.directional_light_position = glm::vec4(transform.position, 0.0f);
			global_info.view_proj_light = _camera->GetProj() * glm::translate(glm::mat4(1.0f), transform.position);
			break;
		}

		// Renderable Objects
		memcpy(global_data, &global_info, sizeof(GlobalDescriptorData));
		vmaUnmapMemory(_allocator, global_descriptor_buffer.allocation);

		// Bind object data storage buffer
		void* objectData;
		vmaMapMemory(_allocator, object_descriptor_buffer[n_frame].allocation, &objectData);
		ObjectData* s_data = (ObjectData*)(objectData);
		ObjectData obj_info = {};

		auto view = scene->Registry()->view<Transform, RenderableObject>();

		uint32_t ctr = 0;
		for (auto entity : view) {
			auto& game_object = Entity(entity, scene);
			Transform& transform = game_object.GetComponent<Transform>();

			glm::mat4 model(1.0f);
			glm::vec3 pos = transform.position;

			model = glm::translate(model, pos);
			model = glm::rotate(model, (float)glm::radians(glm::sin(SDL_GetTicks() * 0.0001) * 360), glm::vec3(0.4f, 0.2f, 0.4f));
			model = glm::scale(model, transform.scale);

			obj_info.model_matrix = model;
			s_data[ctr] = obj_info;

			ctr++;
		}
		vmaUnmapMemory(_allocator, object_descriptor_buffer[n_frame].allocation);

		// Bind global and object descriptors
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _offscreen_render_pipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _default_pipeline_layout, 0, 1, &global_descriptor_set, 0, nullptr);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _default_pipeline_layout, 1, 1, &object_descriptor_set[n_frame], 0, nullptr);

		// Draw Indirect
		static bool recorded = false;
		if (!recorded)
		{
			for (int i = 0; i < frame_overlaps; i++) {
				VkDrawIndexedIndirectCommand* draw_commands;
				vmaMapMemory(_allocator, _frame_data[i].indirect_buffer.allocation, (void**)&draw_commands);

				// Encode draw commands for each renderable ahead of time
				ctr = 0;
				for (auto entity : view) {
					auto& renderable = Entity(entity, scene).GetComponent<RenderableObject>();
					draw_commands[ctr].vertexOffset = 0;
					draw_commands[ctr].indexCount = renderable.mesh->_index_count;
					draw_commands[ctr].instanceCount = 1;
					draw_commands[ctr].firstInstance = ctr;
					ctr++;
				}
				vmaUnmapMemory(_allocator, _frame_data[i].indirect_buffer.allocation);
			}
			recorded = true;
		}

		// Sort rederables into batches
		std::vector<IndirectBatch> batches = GetRenderablesAsBatches(scene);

		for (auto& batch : batches) {
			// Bind batch vertex and index buffer
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(cmd, 0, 1, &batch.mesh->_buffer.buffer, &offset);

			if (batch.mesh->_index_count > 0) {
				vkCmdBindIndexBuffer(cmd, batch.mesh->_index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
			}

			// Bind batch material
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _default_pipeline_layout, 2, 1, &batch.material->texture_set, 0, nullptr);

			// Draw batch
			VkDeviceSize batch_stride = sizeof(VkDrawIndexedIndirectCommand);
			uint32_t indirect_offset = batch.first * batch_stride;

			vkCmdDrawIndexedIndirect(cmd, _frame_data[n_frame].indirect_buffer.buffer, indirect_offset, batch.count, batch_stride);
		}
	}

	bool Renderer::LoadShaderModule(const char* path, VkShaderModule* module)
	{
		std::vector<char> shader_code;
		util::read_file(path, shader_code);

		if (shader_code.data() == nullptr) {
			return false;
		}

		VkShaderModuleCreateInfo shader_info = {};
		shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_info.pNext = nullptr;

		shader_info.codeSize = shader_code.size();
		shader_info.pCode = reinterpret_cast<const uint32_t*>(shader_code.data());
		shader_info.flags = 0;

		if (vkCreateShaderModule(_device.device, &shader_info, nullptr, module) != VK_SUCCESS) {
			return false;
		}

		return true;
	}

	void Renderer::ImmediateSubmit(std::function<void(VkCommandBuffer)>&& fn)
	{
		VkCommandBuffer cmd = _upload_context.command_buffer;

		VkCommandBufferBeginInfo begin_info = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		vkBeginCommandBuffer(cmd, &begin_info);

		// Execute passed commands
		fn(cmd);

		vkEndCommandBuffer(cmd);

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = nullptr;

		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &_upload_context.command_buffer;
		vkQueueSubmit(_queues.graphics, 1, &submit_info, _upload_context.upload_fence);

		vkWaitForFences(_device.device, 1, &_upload_context.upload_fence, VK_TRUE, time_out);
		vkResetFences(_device.device, 1, &_upload_context.upload_fence);

		// Reset command pool
		vkResetCommandPool(_device.device, _upload_context.command_pool, 0);
	}

	AllocatedBuffer Renderer::CreateBuffer(size_t size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage, VkMemoryPropertyFlags memory_props)
	{
		//AllocatedBuffer buffer;
		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.pNext = nullptr;

		buffer_info.usage = buffer_usage;
		buffer_info.size = (VkDeviceSize)size;

		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.usage = memory_usage;
		alloc_info.requiredFlags = memory_props;

		AllocatedBuffer a_buffer;
		vmaCreateBuffer(_allocator, &buffer_info, &alloc_info, &a_buffer.buffer, &a_buffer.allocation, nullptr);

		return a_buffer;
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

		// Create swapchain depth attachment
		VkExtent3D depth_extent = {};
		depth_extent.width = _window->GetWidth();
		depth_extent.height = _window->GetHeight();
		depth_extent.depth = 1;

		_depth_image_format = VK_FORMAT_D32_SFLOAT;
		VkImageCreateInfo depth_image_info = vkinit::image_create_info(_depth_image_format, depth_extent, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

		VmaAllocationCreateInfo depth_alloc_info = {};
		depth_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		depth_alloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		vmaCreateImage(_allocator, &depth_image_info, &depth_alloc_info, &_depth_image.image, &_depth_image.allocation, nullptr);

		VkImageViewCreateInfo depth_image_view_info = vkinit::image_view_create_info(_depth_image.image, _depth_image_format, VK_IMAGE_ASPECT_DEPTH_BIT);
		VK_CHECK(vkCreateImageView(_device.device, &depth_image_view_info, nullptr, &_depth_image_view));

		_deletion_queue.Push([=]() {
			vmaDestroyImage(_allocator, _depth_image.image, _depth_image.allocation);
			vkDestroyImageView(_device.device, _depth_image_view, nullptr);
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

		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depth_attachment = {};
		depth_attachment.format = _depth_image_format;
		depth_attachment.flags = 0;
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		std::vector<VkAttachmentDescription> attachments = { color_attachment, depth_attachment };

		VkAttachmentReference color_attachment_reference = {};
		color_attachment_reference.attachment = 0;
		color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_reference = {};
		depth_attachment_reference.attachment = 1;
		depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Create subpasses
		VkSubpassDescription main_subpass = {};
		main_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		main_subpass.colorAttachmentCount = 1;
		main_subpass.pColorAttachments = &color_attachment_reference;

		main_subpass.pDepthStencilAttachment = &depth_attachment_reference;

		//Create dependencies
		VkSubpassDependency color_dependency = {};

		// ~ after subpass external - outputs to color attachment - using color_attachment write memory access
		color_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		color_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		color_dependency.srcAccessMask = 0;

		// ~ before this subpass - outputs to color attachment - using color_attachment write memory access
		color_dependency.dstSubpass = 0;
		color_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		color_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkSubpassDependency depth_dependency = {};

		// ~ after subpass external - tests fragments - using depth_attachment write memory access
		depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depth_dependency.srcAccessMask = 0;

		// ~ before this subpass - outputs to depth attachment - using depth_attachment write memory access
		depth_dependency.dstSubpass = 0;
		depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::vector<VkSubpassDependency> dependencies = { color_dependency, depth_dependency };

		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.pNext = nullptr;

		render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		render_pass_info.pAttachments = attachments.data();

		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &main_subpass;

		render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
		render_pass_info.pDependencies = dependencies.data();

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

			std::vector<VkImageView> attachments = { _swapchain_image_views[i], _depth_image_view };
			frame_buffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
			frame_buffer_info.pAttachments = attachments.data();

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
		fence_info.flags = 0;
		vkCreateFence(_device.device, &fence_info, nullptr, &_upload_context.upload_fence);
		_deletion_queue.Push([=]() {
			vkDestroyFence(_device.device, _upload_context.upload_fence, nullptr);
			});
	}

	void Renderer::InitPipelines()
	{
		vkutil::PipelineBuilder pipeline_builder = {};

		// Pipeline view port
		pipeline_builder._scissor.extent = { _window->GetWidth(), _window->GetHeight() };
		pipeline_builder._scissor.offset = { 0, 0 };

		pipeline_builder._viewport.x = 0.0f;
		pipeline_builder._viewport.y = 0.0f;
		pipeline_builder._viewport.width = (float)(_window->GetWidth());
		pipeline_builder._viewport.height = (float)(_window->GetHeight());
		pipeline_builder._viewport.minDepth = 0.0f;
		pipeline_builder._viewport.maxDepth = 1.0f;

		// Shaders modules
		bool loaded = false;
		VkShaderModule vertex_shader_module, fragment_shader_module = {};
		loaded = LoadShaderModule("Assets/Shaders/default_shader.vert.spv", &vertex_shader_module);
		loaded = LoadShaderModule("Assets/Shaders/default_shader.frag.spv", &fragment_shader_module);

		pipeline_builder._shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader_module));
		pipeline_builder._shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader_module));

		// Vertex input (Primitives and Vertex Input Descriptions
		pipeline_builder._input_assembly = vkinit::pipeline_input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		VertexInputDescription& vb_desc = Vertex::GetDescription();

		pipeline_builder._vertex_input_info = vkinit::pipeline_vertex_input_state_create_info();

		pipeline_builder._vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vb_desc.bindings.size());
		pipeline_builder._vertex_input_info.pVertexBindingDescriptions = vb_desc.bindings.data();

		pipeline_builder._vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vb_desc.attributes.size());
		pipeline_builder._vertex_input_info.pVertexAttributeDescriptions = vb_desc.attributes.data();

		// Graphics Settings
		pipeline_builder._rasterizer = vkinit::pipeline_rasterization_state_create_info(VK_POLYGON_MODE_FILL);
		pipeline_builder._multisampling = vkinit::pipeline_mulitisample_state_create_info();
		pipeline_builder._color_blend_attachment = vkinit::pipeline_color_blend_attachment_state();
		pipeline_builder._depth_stencil = vkinit::pipeline_depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

		// Create pipeline layout
		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();

		// DEMO: Added Shadow Descriptor
		std::vector<VkDescriptorSetLayout> descriptor_layouts = { global_descriptor_set_layout , object_descriptor_set_layout, _default_image_set_layout, _shadow_map_descriptor_layout };
		pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_layouts.size());
		pipeline_layout_info.pSetLayouts = descriptor_layouts.data();

		VK_CHECK(vkCreatePipelineLayout(_device.device, &pipeline_layout_info, nullptr, &_default_pipeline_layout));
		pipeline_builder._pipeline_layout = _default_pipeline_layout;

		_default_pipeline = pipeline_builder.Build(_device.device, _render_pass);

		if (_default_pipeline == VK_NULL_HANDLE) {
			printf("Failed to create pipeline!");
		}

		// Clean Up
		vkDestroyShaderModule(_device.device, fragment_shader_module, nullptr);
		vkDestroyShaderModule(_device.device, vertex_shader_module, nullptr);
		_deletion_queue.Push([=]() {
			vkDestroyPipelineLayout(_device.device, _default_pipeline_layout, nullptr);
			vkDestroyPipeline(_device.device, _default_pipeline, nullptr);
			});


		// DEMO: 
		InitOffScreenPipeline();
	}

	void Renderer::InitDescriptors()
	{
		// Init descriptor resource managers
		_descriptor_allocator = std::make_unique<vkutil::DescriptorAllocator>();
		_descriptor_allocator->Init(_device.device);

		_descriptor_layout_cache = std::make_unique<vkutil::DescriptorLayoutCache>();
		_descriptor_layout_cache->Init(_device.device);

		// Init descriptor resources (i.e buffers, images) DescriptorInfos

		// ~ Global
		{
			size_t buffer_size = PadToUniformBufferSize(sizeof(GlobalDescriptorData));
			global_descriptor_buffer = CreateBuffer(sizeof(GlobalDescriptorData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

			GlobalDescriptorData global_descriptor_data = {};
			global_descriptor_data.view_proj = _camera->GetViewProj();
			global_descriptor_data.directional_light_position = glm::vec4(0.0f);

			void* data;
			vmaMapMemory(_allocator, global_descriptor_buffer.allocation, &data);
			memcpy(data, &global_descriptor_data, sizeof(GlobalDescriptorData));
			vmaUnmapMemory(_allocator, global_descriptor_buffer.allocation);

			// Describe Buffer
			VkDescriptorBufferInfo global_descriptor_buffer_info = {};
			global_descriptor_buffer_info.buffer = global_descriptor_buffer.buffer;
			global_descriptor_buffer_info.offset = 0;
			global_descriptor_buffer_info.range = sizeof(GlobalDescriptorData);

			// Build descriptors
			vkutil::DescriptorBuilder::Begin(_descriptor_layout_cache.get(), _descriptor_allocator.get())
				.BindBuffer(0, &global_descriptor_buffer_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
				.Build(&global_descriptor_set, &global_descriptor_set_layout);

			// Clean descriptor resources
			_deletion_queue.Push([=]() {
				vmaDestroyBuffer(_allocator, global_descriptor_buffer.buffer, global_descriptor_buffer.allocation);
				});
		}

		// ~ Object Data
		{
			object_descriptor_buffer.resize(frame_overlaps);
			object_descriptor_set.resize(frame_overlaps);
			for (uint32_t i = 0; i < frame_overlaps; i++) {

				// Create buffer to store per object data per frame
				//size_t buffer_size = PadToUniformBufferSize(sizeof(ObjectData) * max_objects);
				size_t buffer_size = sizeof(ObjectData) * max_objects;
				object_descriptor_buffer[i] = CreateBuffer(buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

				// Create data for buffer
				std::vector<ObjectData> object_data(max_objects);
				for (int i = 0; i < max_objects; i++) {
					glm::vec3 pos = glm::vec3(0.0, i * 0.2f, 0.2);

					glm::mat4 model(1.0f);
					model = glm::translate(model, pos);
					model = glm::scale(model, glm::vec3(0.25f));
					object_data[i].model_matrix = model;
					object_data[i].color = glm::vec4(pos, 1.0f);
				}

				// Copy to GPU
				void* data;
				vmaMapMemory(_allocator, object_descriptor_buffer[i].allocation, &data);
				ObjectData* object_dataSSBO = (ObjectData*)data;
				for (uint32_t i = 0; i < max_objects; i++) {
					object_dataSSBO[i] = object_data[i];
				}
				vmaUnmapMemory(_allocator, object_descriptor_buffer[i].allocation);

				// Describe Buffer
				VkDescriptorBufferInfo object_descriptor_buffer_info = {};
				object_descriptor_buffer_info.buffer = object_descriptor_buffer[i].buffer;
				object_descriptor_buffer_info.offset = 0;
				object_descriptor_buffer_info.range = sizeof(ObjectData) * max_objects;

				// Build descriptors
				vkutil::DescriptorBuilder::Begin(_descriptor_layout_cache.get(), _descriptor_allocator.get())
					.BindBuffer(0, &object_descriptor_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
					.Build(&object_descriptor_set[i], &object_descriptor_set_layout);

				// Clean descriptor resources
				_deletion_queue.Push([=]() {
					vmaDestroyBuffer(_allocator, object_descriptor_buffer[i].buffer, object_descriptor_buffer[i].allocation);
					});
			}
		}
	}

	size_t Renderer::PadToUniformBufferSize(size_t original_size)
	{
		// Calculate required alignment based on minimum device offset alignment
		size_t min_ubo_allignment = _physical_device_props.limits.minUniformBufferOffsetAlignment;
		size_t aligned_size = original_size;

		if (min_ubo_allignment > 0) {
			aligned_size = (aligned_size + min_ubo_allignment - 1) & ~(min_ubo_allignment - 1);
		}
		return aligned_size;
	}

	void Renderer::Begin(Scene* scene)
	{
		// Get current frame index, current frame data, current cmd bufffer
		auto& frame = _frame_data[GetCurrentFrame()];
		VkCommandBuffer cmd = frame.command_buffer;

		// Wait till render fence has been flagged
		VK_CHECK(vkWaitForFences(_device.device, 1, &frame.render_fence, VK_TRUE, time_out));
		vkResetFences(_device.device, 1, &frame.render_fence);

		// Get current image to render to 
		// Make sure image has been acquired before submitting
		vkAcquireNextImageKHR(_device.device, _swapchain, time_out, frame.present_semaphore, VK_NULL_HANDLE, &_current_swapchain_image);

		// ~ Begin Recording 
		VK_CHECK(vkResetCommandBuffer(cmd, 0));
		VkCommandBufferBeginInfo cmd_begin_info = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begin_info));

		VkRect2D area = {};
		area.extent = { _window->GetWidth(), _window->GetHeight() };
		area.offset = { 0,0 };

		VkClearValue depth_value = {};
		depth_value.depthStencil = { 1.0f, 0 };

		// DEMO: Draw offscreen frame buffer for shadow map 
		VkRenderPassBeginInfo offscreen_render_pass_begin_info = vkinit::render_pass_begin_info(_offscreen_framebuffer, _offscreen_render_pass, area, &depth_value, 1);
		vkCmdBeginRenderPass(cmd, &offscreen_render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		DrawShadowMap(cmd, scene);
		vkCmdEndRenderPass(cmd);

		// Draw scene

		// clear values are per attachment 
		VkClearValue clear_value = {};
		clear_value.color = { (float)sin(_frame_count / 120.0f), 0.1f, 0.1f };
		depth_value.depthStencil = { 1.0f, 0 };
		VkClearValue clear_values[2] = { clear_value, depth_value };

		VkRenderPassBeginInfo render_pass_begin_info = vkinit::render_pass_begin_info(_framebuffers[_current_swapchain_image], _render_pass, area, clear_values, 2);
		vkCmdBeginRenderPass(cmd, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		// Draw all renderable objects
		DrawObjects(cmd, scene);

		// Set up ImGui to Draw
		ImGui::Render();
	}

	void Renderer::End()
	{
		auto& frame = _frame_data[GetCurrentFrame()];
		VkCommandBuffer cmd = frame.command_buffer;

		// TODO: Move to UI render pass
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
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

		present_info.pImageIndices = &_current_swapchain_image;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &_swapchain;

		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &frame.render_semaphore;

		VK_CHECK(vkQueuePresentKHR(_queues.graphics, &present_info));

		// Update frame counter
		++_frame_count;
	}
}