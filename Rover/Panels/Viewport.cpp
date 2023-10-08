#include "Viewport.h"
#include <Renderer/Vulkan/VulkanUtils.h>

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <Core/Input.h>

mrs::Viewport::Viewport(EditorLayer* editor_layer, const std::string& name, IRenderPipelineLayer* render_pipeline_layer)
	:IPanel(editor_layer, name)
{
	_renderer = render_pipeline_layer->GetRenderer();

	// Create view port sampler
	VkSamplerCreateInfo sampler_info = vkinit::SamplerCreateInfo(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	VK_CHECK(vkCreateSampler(_renderer->Device().device, &sampler_info, nullptr, &_viewport_sampler));

	// Create Imgui descriptor set
	const auto& offscreen_image_views = _renderer->OffScreenImageViews();
	for (uint32_t i = 0; i < offscreen_image_views.size(); i++)
	{
		_viewport_descriptor_sets.push_back(ImGui_ImplVulkan_AddTexture(_viewport_sampler, offscreen_image_views[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
	}

	_renderer->DeletionQueue().Push([&]() {
		for (VkDescriptorSet viewport_descriptor : _viewport_descriptor_sets)
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

	auto viewport_size = ImGui::GetContentRegionAvail();
	auto viewport_pos_s = ImGui::GetCursorScreenPos();

	auto mouse_pos_s = ImGui::GetMousePos();
	glm::vec2 mouse_pos_v = glm::vec2(mouse_pos_s.x, mouse_pos_s.y) - glm::vec2(viewport_pos_s.x, viewport_pos_s.y);
	
	// TODO: Get aspect from application
	glm::vec2 aspects = glm::vec2(1600.0f, 900.0f);
	glm::vec2 mouse_pos_app_space = glm::vec2(mouse_pos_v.x / viewport_size.x, mouse_pos_v.y / viewport_size.y) * aspects;

	Input::x = mouse_pos_app_space.x;
	Input::y = mouse_pos_app_space.y;

	// viewport_size.x = aspects.x * 0.75f;
	// viewport_size.y = aspects.y * 0.75f;
	ImGui::Image(_viewport_descriptor_sets[frame], viewport_size);
	ImGui::End();
}
