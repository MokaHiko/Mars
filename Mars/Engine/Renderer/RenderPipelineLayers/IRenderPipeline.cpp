#include "IRenderPipeline.h"

mrs::IRenderPipeline::IRenderPipeline(const std::string &name) 
	: _name(name){}

mrs::IRenderPipeline::IRenderPipeline(const std::string &name,
                                      Renderer *renderer,
                                      VkRenderPass _render_pass,
                                      VkFormat _render_pass_format)
    : _name(name) {
  _renderer = renderer;

  _render_pass = _render_pass;
  _render_pass_format = _render_pass_format;
};
