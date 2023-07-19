#include "ShadowRenderPipeline.h"

#include "ECS/Components/Components.h"
#include "Renderer/Vulkan/VulkanInitializers.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


void mrs::ShadowRenderPipeline::OnPreRenderPass(VkCommandBuffer cmd) 
{

}

void mrs::ShadowRenderPipeline::InitDescriptors()
{
    VkDescriptorImageInfo shadow_map_image_info = {};
    shadow_map_image_info.sampler = _shadow_map_sampler;
    shadow_map_image_info.imageView = _offscreen_depth_image_view;
    shadow_map_image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    vkutil::DescriptorBuilder::Begin(_renderer->_descriptor_layout_cache.get(), _renderer->_descriptor_allocator.get())
        .BindImage(0, &shadow_map_image_info, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build(&_shadow_map_descriptor, &_shadow_map_descriptor_layout);
}

void mrs::ShadowRenderPipeline::InitPipelineLayout() 
{
	VkPipelineLayoutCreateInfo info = {};
}


void mrs::ShadowRenderPipeline::InitOffScreenPipeline() {
  vkutil::PipelineBuilder pipeline_builder = {};

  // Pipeline view port
  pipeline_builder._scissor.extent = {_window->GetWidth(),
                                      _window->GetHeight()};
  pipeline_builder._scissor.offset = {0, 0};

  pipeline_builder._viewport.x = 0.0f;
  pipeline_builder._viewport.y = 0.0f;
  pipeline_builder._viewport.width = (float)(_window->GetWidth());
  pipeline_builder._viewport.height = (float)(_window->GetHeight());
  pipeline_builder._viewport.minDepth = 0.0f;
  pipeline_builder._viewport.maxDepth = 1.0f;

  // Shaders modules
  bool loaded = false;
  VkShaderModule vertex_shader_module;
  loaded = _renderer->LoadShaderModule(
      "Assets/Shaders/offscreen_shader.vert.spv", &vertex_shader_module);
  pipeline_builder._shader_stages.push_back(
      vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT,
                                                vertex_shader_module));

  // Vertex input (Primitives and Vertex Input Descriptions
  pipeline_builder._input_assembly =
      vkinit::pipeline_input_assembly_state_create_info(
          VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  VertexInputDescription &vb_desc = Vertex::GetDescription();

  pipeline_builder._vertex_input_info =
      vkinit::pipeline_vertex_input_state_create_info();

  pipeline_builder._vertex_input_info.vertexBindingDescriptionCount =
      static_cast<uint32_t>(vb_desc.bindings.size());
  pipeline_builder._vertex_input_info.pVertexBindingDescriptions =
      vb_desc.bindings.data();

  pipeline_builder._vertex_input_info.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(vb_desc.attributes.size());
  pipeline_builder._vertex_input_info.pVertexAttributeDescriptions =
      vb_desc.attributes.data();

  // Disable cull so all faces contribute to shadows
  pipeline_builder._rasterizer =
      vkinit::pipeline_rasterization_state_create_info(VK_POLYGON_MODE_FILL);
  pipeline_builder._rasterizer.cullMode = VK_CULL_MODE_NONE;

  pipeline_builder._multisampling =
      vkinit::pipeline_mulitisample_state_create_info();
  pipeline_builder._color_blend_attachment = {};
  pipeline_builder._depth_stencil = vkinit::pipeline_depth_stencil_create_info(
      true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

  // Create pipeline layout
  VkPipelineLayoutCreateInfo pipeline_layout_info =
      vkinit::pipeline_layout_create_info();

  std::vector<VkDescriptorSetLayout> descriptor_layouts = {
      _global_descriptor_set_layout, _object_descriptor_set_layout,
      _default_image_set_layout};
  pipeline_layout_info.setLayoutCount =
      static_cast<uint32_t>(descriptor_layouts.size());
  pipeline_layout_info.pSetLayouts = descriptor_layouts.data();

  pipeline_builder._pipeline_layout = _offscreen_pipeline_layout;

  _offscreen_render_pipeline =
      pipeline_builder.Build(_device->device, _offscreen_render_pass, true);

  if (_offscreen_render_pipeline == VK_NULL_HANDLE) {
    printf("Failed to create pipeline!");
  }

  // Clean Up
  vkDestroyShaderModule(_device->device, vertex_shader_module, nullptr);
  _renderer->GetDeletionQueue().Push([=]() {
    vkDestroyPipeline(_device->device, _offscreen_render_pipeline, nullptr);
  });
}

void mrs::ShadowRenderPipeline::DrawShadowMap(VkCommandBuffer cmd, Scene *scene)
{
	
}

