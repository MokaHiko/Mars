#include "Viewport.h"
#include <Renderer/Vulkan/VulkanUtils.h>

#include <imgui.h>
#include <imgui_impl_vulkan.h>

mrs::Viewport::Viewport(EditorLayer *editor_layer, const std::string &name, IRenderPipelineLayer *render_pipeline_layer)
	:IPanel(editor_layer, name)
{
	_renderer =  render_pipeline_layer->GetRenderer();

    // Create view port sampler
    VkSamplerCreateInfo sampler_info = vkinit::SamplerCreateInfo(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	VK_CHECK(vkCreateSampler(_renderer->Device().device, &sampler_info, nullptr, &_viewport_sampler));

	// Create Imgui descriptor set
	const auto& offscreen_image_views = _renderer->OffScreenImageViews();
	for(uint32_t i = 0; i < offscreen_image_views.size(); i++)
	{
		_viewport_descriptor_sets.push_back(ImGui_ImplVulkan_AddTexture(_viewport_sampler, offscreen_image_views[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
	}

	_renderer->DeletionQueue().Push([&](){
		for(VkDescriptorSet viewport_descriptor : _viewport_descriptor_sets)
		{
			ImGui_ImplVulkan_RemoveTexture(viewport_descriptor);
		}

    	vkDestroySampler(_renderer->Device().device, _viewport_sampler, nullptr);
	});
}

mrs::Viewport::~Viewport() {}

void mrs::Viewport::Draw() 
{
  uint32_t frame = _renderer->CurrentFrame();
  ImGui::Begin("Viewport");

  ImGui::Image(_viewport_descriptor_sets[frame],
               ImGui::GetContentRegionAvail());
  ImGui::End();
}
