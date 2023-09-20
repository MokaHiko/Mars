#include "IRenderPipeline.h"
#include "Core/Memory.h"

mrs::IRenderPipeline::IRenderPipeline(const std::string& name)
  : _name(name) {}

mrs::IRenderPipeline::IRenderPipeline(const std::string& name,
  Renderer* renderer,
  VkRenderPass _render_pass,
  VkFormat _render_pass_format)
  : _name(name) {
  _renderer = renderer;

  _render_pass = _render_pass;
  _render_pass_format = _render_pass_format;
}

Ref<mrs::ShaderEffect> mrs::IRenderPipeline::Effect()
{
  MRS_ASSERT(_descriptor_set_layout != VK_NULL_HANDLE);
  MRS_ASSERT(_descriptor_set != VK_NULL_HANDLE);

  if(!_shader_effect)
  {
    _shader_effect = CreateRef<ShaderEffect>();
    _shader_effect->render_pipeline = this;
    _shader_effect->descriptor_set = _descriptor_set;
    _shader_effect->descriptor_set_layout = _descriptor_set_layout;
  }

  return _shader_effect;
}