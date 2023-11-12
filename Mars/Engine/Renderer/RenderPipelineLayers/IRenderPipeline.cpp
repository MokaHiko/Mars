#include "IRenderPipeline.h"
#include "Core/Memory.h"

#include <spirv_reflect/spirv_reflect.h>

#include "Renderer/Vulkan/VulkanInitializers.h"

void mrs::IRenderPipeline::ParseDescriptorSetFromSpirV(const void* spirv_code, size_t spirv_nbytes, VkShaderStageFlagBits stage)
{
  // Generate reflection data for a shader
  SpvReflectShaderModule module;
  SpvReflectResult result = spvReflectCreateShaderModule(spirv_nbytes, spirv_code, &module);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  // Get descriptor bindings
  uint32_t count = 0;
  result = spvReflectEnumerateDescriptorSets(&module, &count, NULL);
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  std::vector<SpvReflectDescriptorSet*> r_descriptor_sets(count);
  result = spvReflectEnumerateDescriptorSets(&module, &count, r_descriptor_sets.data());
  assert(result == SPV_REFLECT_RESULT_SUCCESS);

  for (uint32_t i = 0; i < r_descriptor_sets.size(); i++)
  {
    MRS_INFO("Descriptor Set: %d", i);
    const SpvReflectDescriptorSet& r_descriptor_set = *r_descriptor_sets[i];

    // Check if descriptor set already exists
    auto it = std::find_if(_required_descriptors.begin(), _required_descriptors.end(), [&](const VulkanDescriptorSet& ds)
    {
      return ds.set == r_descriptor_set.set;
    });

    if (it != _required_descriptors.end())
    {
      for (uint32_t j = 0; j < r_descriptor_set.binding_count; j++)
      {
        const SpvReflectDescriptorBinding& r_descriptor_binding = *r_descriptor_set.bindings[j];
        VkDescriptorType type = static_cast<VkDescriptorType>(r_descriptor_binding.descriptor_type);

        MRS_INFO("binding: %d, descriptor type: %d", r_descriptor_binding.binding, r_descriptor_binding.descriptor_type);
        it->AddBinding(r_descriptor_binding.binding, type, stage);
      };
    }
    else
    {
      VulkanDescriptorSet descriptor_set = {};
      descriptor_set.shader_stage = stage;
      descriptor_set.set = r_descriptor_set.set;

      for (uint32_t j = 0; j < r_descriptor_set.binding_count; j++)
      {
        const SpvReflectDescriptorBinding& r_descriptor_binding = *r_descriptor_set.bindings[j];
        VkDescriptorType type = static_cast<VkDescriptorType>(r_descriptor_binding.descriptor_type);

        MRS_INFO("binding: %d, descriptor type: %d", r_descriptor_binding.binding, r_descriptor_binding.descriptor_type);
        descriptor_set.AddBinding(r_descriptor_binding.binding, type, stage);
      };

      _required_descriptors.push_back(descriptor_set);
    }
  }

  // Clean up
  spvReflectDestroyShaderModule(&module);
}

mrs::IRenderPipeline::IRenderPipeline(const std::string& name)
  : _name(name) {}

mrs::IRenderPipeline::IRenderPipeline(const std::string& name, VkRenderPass render_pass)
  : _name(name)
{
  _render_pass = render_pass;
}

Ref<mrs::ShaderEffect> mrs::IRenderPipeline::Effect()
{
  MRS_ASSERT(_descriptor_set_layout != VK_NULL_HANDLE);
  MRS_ASSERT(_descriptor_set != VK_NULL_HANDLE);

  if (!_shader_effect)
  {
    _shader_effect = CreateRef<ShaderEffect>();
    _shader_effect->render_pipeline = this;
  }

  return _shader_effect;
}

void mrs::IRenderPipeline::PushShader(Ref<Shader> shader)
{
  MRS_INFO("Shader: %s", shader->name.c_str());
  _shaders.push_back(shader);

  // Push to required descriptor sets vector
  ParseDescriptorSetFromSpirV(shader->byte_code.data(), shader->byte_code.size(), shader->stage);
}

void mrs::IRenderPipeline::BuildPipeline()
{
  // Build descriptor set layouts
  std::vector<VkDescriptorSetLayout> descriptor_layouts;
  for (VulkanDescriptorSet& descriptor : _required_descriptors)
  {
    // Build descriptor set layouts
    vkutil::DescriptorBuilder builder = vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator());
    for (auto it = descriptor.bindings.begin(); it != descriptor.bindings.end(); it++)
    {
      uint32_t binding = it->first;
      VkDescriptorType type = it->second;
      if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
      {
        builder.BindBuffer(binding, nullptr, type, descriptor.shader_stage);
      }
      else if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
      {
        builder.BindImage(binding, nullptr, type, descriptor.shader_stage);
      }
    }

    builder.Build(nullptr, &descriptor.descriptor_set_layout);
    descriptor_layouts.push_back(descriptor.descriptor_set_layout);
  }

  // Build pipeline layout
  VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::PipelineLayoutCreateInfo();
  pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_layouts.size());
  pipeline_layout_info.pSetLayouts = descriptor_layouts.data();
  VK_CHECK(vkCreatePipelineLayout(_device->device, &pipeline_layout_info, nullptr, &_pipeline_layout));

  // Build pipeline
  vkutil::PipelineBuilder pipeline_builder = {};

  // Pipeline view port
  pipeline_builder._scissor.extent = { _window->GetWidth(),
                                      _window->GetHeight() };
  pipeline_builder._scissor.offset = { 0, 0 };

  pipeline_builder._viewport.x = 0.0f;
  pipeline_builder._viewport.y = 0.0f;
  pipeline_builder._viewport.width = (float)(_window->GetWidth());
  pipeline_builder._viewport.height = (float)(_window->GetHeight());
  pipeline_builder._viewport.minDepth = 0.0f;
  pipeline_builder._viewport.maxDepth = 1.0f;

  // Shaders modules
  for (const Ref<Shader>& shader : _shaders)
  {
    pipeline_builder._shader_stages.push_back(vkinit::PipelineShaderStageCreateInfo(shader->stage, shader->shader_module));
  }

  // Vertex input (Primitives and Vertex Input Descriptions
  pipeline_builder._input_assembly = vkinit::PipelineInputAssemblyStateCreateInfo(_render_pipeline_settings.primitive_topology);
  VertexInputDescription& vb_desc = Vertex::GetDescription();

  pipeline_builder._vertex_input_info = vkinit::PipelineVertexInputStateCreateInfo();

  pipeline_builder._vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vb_desc.bindings.size());
  pipeline_builder._vertex_input_info.pVertexBindingDescriptions = vb_desc.bindings.data();

  pipeline_builder._vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vb_desc.attributes.size());
  pipeline_builder._vertex_input_info.pVertexAttributeDescriptions = vb_desc.attributes.data();

  // Graphics Settings
  pipeline_builder._rasterizer = vkinit::PipelineRasterizationStateCreateInfo(_render_pipeline_settings.polygon_mode);
  pipeline_builder._multisampling = vkinit::PipelineMultisampleStateCreateInfo();
  pipeline_builder._color_blend_attachment = vkinit::PipelineColorBlendAttachmentState();
  pipeline_builder._depth_stencil = vkinit::PipelineDepthStencilStateCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

  if(_render_pipeline_settings.tesselation_control_points > 0)
  {
    pipeline_builder._tesselation_state = vkinit::PipelineTesselationStateCreateInfo(_render_pipeline_settings.tesselation_control_points);
  }

  pipeline_builder._pipeline_layout = _pipeline_layout;

  _pipeline = pipeline_builder.Build(_device->device, _render_pass);

  if (_pipeline == VK_NULL_HANDLE)
  {
    printf("Failed to create pipeline!");
  }

  // Clean Up
  _renderer->DeletionQueue().Push([=]()
  {
    vkDestroyPipelineLayout(_device->device, _pipeline_layout, nullptr);
    vkDestroyPipeline(_device->device, _pipeline, nullptr);
  });
}

