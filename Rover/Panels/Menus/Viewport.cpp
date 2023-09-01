#include "Viewport.h"
#include <Renderer/Vulkan/VulkanUtils.h>

#include <imgui.h>
#include <imgui_impl_vulkan.h>

Ref<mrs::Renderer> mrs::Viewport::_renderer = nullptr;
VkSampler mrs::Viewport::_viewport_sampler = VK_NULL_HANDLE;
std::vector<VkDescriptorSet> mrs::Viewport::_viewport_descriptor_sets;

void mrs::Viewport::Init(IRenderPipelineLayer* render_pipeline_layer)
{
	_renderer =  render_pipeline_layer->GetRenderer();

    // Create view port sampler
    VkSamplerCreateInfo sampler_info = vkinit::SamplerCreateInfo(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	VK_CHECK(vkCreateSampler(_renderer->GetDevice().device, &sampler_info, nullptr, &_viewport_sampler));

	// Create Imgui descriptor set
	const auto& offscreen_image_views = _renderer->GetOffScreenImageViews();
	for(uint32_t i = 0; i < offscreen_image_views.size(); i++)
	{
		_viewport_descriptor_sets.push_back(ImGui_ImplVulkan_AddTexture(_viewport_sampler, offscreen_image_views[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
	}

	_renderer->GetDeletionQueue().Push([=](){
    	vkDestroySampler(_renderer->GetDevice().device, _viewport_sampler, nullptr);
	});
}

void mrs::Viewport::Draw() 
{
	uint32_t frame = _renderer->GetCurrentFrame();
	ImGui::Begin("Viewport");
	
	ImGui::Image(_viewport_descriptor_sets[frame], ImGui::GetContentRegionAvail());
	ImGui::End();
}
