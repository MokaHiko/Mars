#include "PostProcessingRenderPipeline.h"
#include "Core/ResourceManager.h"

#include "Renderer/Vulkan/VulkanInitializers.h"

#include "Renderer/Vulkan/VulkanTexture.h"
#include "Renderer/Vulkan/VulkanMesh.h"

void mrs::PostProcessingRenderPipeline::Init()
{
	_screen_quad = Mesh::Get("quad");

	// Create screen sampler
	VkSamplerCreateInfo sampler_info = vkinit::sampler_create_info(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	VK_CHECK(vkCreateSampler(_device->device, &sampler_info, nullptr, &_screen_sampler));

	InitDescriptors();
	InitPostProcessPipeline();
}

void mrs::PostProcessingRenderPipeline::InitPostProcessPipeline()
{
	vkutil::PipelineBuilder builder = {};

	// Viewport & scissor
	builder._scissor.extent = { _window->GetWidth(), _window->GetHeight() };
	builder._scissor.offset = { 0, 0 };

	builder._viewport.x = 0.0f;
	builder._viewport.y = 0.0f;
	builder._viewport.width = static_cast<float>(_window->GetWidth());
	builder._viewport.height = static_cast<float>(_window->GetHeight());
	builder._viewport.minDepth = 0.0f;
	builder._viewport.maxDepth = 1.0f;

	// Shader
	VkShaderModule vertex_shader;
	_renderer->LoadShaderModule("assets/shaders/post_process_shader.vert.spv", &vertex_shader);
	VkShaderModule fragment_shader;
	_renderer->LoadShaderModule("assets/shaders/post_process_shader.frag.spv", &fragment_shader);

	builder._shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader));
	builder._shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader));

	// Vertex Input
	VertexInputDescription vertex_desc = Vertex::GetDescription();
	auto bindings = vertex_desc.bindings;
	auto attributes = vertex_desc.attributes;

	builder._vertex_input_info = vkinit::pipeline_vertex_input_state_create_info();
	builder._vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
	builder._vertex_input_info.pVertexBindingDescriptions = bindings.data();
	builder._vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
	builder._vertex_input_info.pVertexAttributeDescriptions = attributes.data();
	builder._input_assembly = vkinit::pipeline_input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	// Graphics Settings
	builder._rasterizer = vkinit::pipeline_rasterization_state_create_info(VK_POLYGON_MODE_FILL);
	builder._multisampling = vkinit::pipeline_mulitisample_state_create_info();
	builder._color_blend_attachment = vkinit::pipeline_color_blend_attachment_state();
	builder._depth_stencil = vkinit::pipeline_depth_stencil_create_info(false, false, VK_COMPARE_OP_ALWAYS);

	// Pipeline layouts
	VkPipelineLayoutCreateInfo layout_info = vkinit::pipeline_layout_create_info();
	std::vector<VkDescriptorSetLayout> set_layouts = { _post_process_descriptor_set_layout };

	layout_info.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
	layout_info.pSetLayouts = set_layouts.data();

	VK_CHECK(vkCreatePipelineLayout(_renderer->GetDevice().device, &layout_info, nullptr, &_post_process_pipeline_layout));

	builder._pipeline_layout = _post_process_pipeline_layout;

	_post_process_pipeline = builder.Build(_renderer->GetDevice().device, _render_pass);

	if (_post_process_pipeline == VK_NULL_HANDLE)
	{
		VK_CHECK(VK_ERROR_UNKNOWN);
	}

	// Clean up
	vkDestroyShaderModule(_renderer->GetDevice().device, vertex_shader, nullptr);
	vkDestroyShaderModule(_renderer->GetDevice().device, fragment_shader, nullptr);
	_renderer->GetDeletionQueue().Push([&]() {
		vkDestroyPipeline(_renderer->GetDevice().device, _post_process_pipeline, nullptr);
		vkDestroyPipelineLayout(_renderer->GetDevice().device, _post_process_pipeline_layout, nullptr);
		});
}

void mrs::PostProcessingRenderPipeline::InitDescriptors()
{
	auto test_image = Texture::Get("chicago_traffic");


	_post_process_descriptor_sets.resize(frame_overlaps);
	for (uint32_t i = 0; i < _post_process_descriptor_sets.size(); i++)
	{
		VkDescriptorImageInfo base_image_info = {};
		base_image_info.sampler = _screen_sampler;
		base_image_info.imageView = _renderer->_offscreen_images_views[i];
		base_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		vkutil::DescriptorBuilder::Begin(_renderer->_descriptor_layout_cache.get(), _renderer->_descriptor_allocator.get())
			.BindImage(0, &base_image_info, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(&_post_process_descriptor_sets[i], &_post_process_descriptor_set_layout);
	}
}

void mrs::PostProcessingRenderPipeline::OnPostRenderpass(VkCommandBuffer cmd)
{
	VkRect2D area = {};
	area.extent = { _window->GetWidth(), _window->GetHeight() };
	area.offset = { 0, 0 };

	// Begin main render pass
	VkClearValue clear_value = {};
	clear_value.color = { 0.0f, 0.0f, 0.0f, 1.0f };

	VkClearValue depth_value = {};
	depth_value.depthStencil = { 1.0f, 0 };

	VkClearValue clear_values[2] = { clear_value, depth_value };

	VkRenderPassBeginInfo render_pass_begin_info = vkinit::render_pass_begin_info(_renderer->GetCurrentFrameBuffer(), _renderer->GetSwapchainRenderPass(), area, clear_values, 2);
	vkCmdBeginRenderPass(cmd, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _post_process_pipeline);

	// Draw scene as quad
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _post_process_pipeline_layout, 0, 1, &_post_process_descriptor_sets[_renderer->GetCurrentFrame()], 0, nullptr);

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &_screen_quad->_buffer.buffer,  &offset);
	vkCmdBindIndexBuffer(cmd, _screen_quad->_index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(cmd, _screen_quad->_index_count, 1, 0, 0, 0);

	vkCmdEndRenderPass(cmd);
}
