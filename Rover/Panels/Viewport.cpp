#include "Viewport.h"
#include <Renderer/Vulkan/VulkanUtils.h>
#include <Renderer/Vulkan/VulkanInitializers.h>

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <Core/Input.h>

#include <Rover.h>
#include <ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>

#include <Math/Math.h>

mrs::Viewport::Viewport(EditorLayer& editor_layer, const std::string& name, IRenderPipelineLayer* render_pipeline_layer)
	:IPanel(editor_layer, name)
{
	_renderer = render_pipeline_layer->GetRenderer();

	// Create view port sampler
	VkSamplerCreateInfo sampler_info = vkinit::SamplerCreateInfo(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	VK_CHECK(vkCreateSampler(_renderer->Device().device, &sampler_info, nullptr, &_viewport_sampler));


	// TODO: Create separate image/image_views and render pass for viewport viewing
	// std::vector<VkImageView> view_port_image_views(_renderer->SwapchainImages().size());
	// for(int i = 0; i < _renderer->SwapchainImages().size(); i++)
	// {
	// 	VkImageCreateInfo view_port_image_info = vkinit::ImageCreateInfo(
	// 	);
	// }
	// for(int i = 0; i < _renderer->SwapchainImages().size(); i++)
	// {
	// 	VkImageViewCreateInfo view_port_image_view_info = vkinit::ImageViewCreateInfo(
	// 		_renderer->SwapchainImages()[i],
	// 		VK_FORMAT_B8G8R8A8_UNORM,
	// 		VK_IMAGE_ASPECT_COLOR_BIT
	// 	);

	// 	vkCreateImageView(_renderer->Device().device, &view_port_image_view_info, nullptr, &view_port_image_views[i]);
	// }

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

	bool is_focused = ImGui::IsWindowFocused();
	if(_focus_state != is_focused)
	{
		_focus_state = is_focused;
		_editor_layer.ToggleInput(is_focused);
	}

	auto viewport_size = ImGui::GetContentRegionAvail();
	auto viewport_pos_s = ImGui::GetCursorScreenPos();

	auto mouse_pos_s = ImGui::GetMousePos();
	glm::vec2 mouse_pos_v = glm::vec2(mouse_pos_s.x, mouse_pos_s.y) - glm::vec2(viewport_pos_s.x, viewport_pos_s.y);
	
	// TODO: Get aspect from application
	glm::vec2 aspects = glm::vec2(1600.0f, 900.0f);
	glm::vec2 mouse_pos_app_space = glm::vec2(mouse_pos_v.x / viewport_size.x, mouse_pos_v.y / viewport_size.y) * aspects;

	Input::x = static_cast<int>(mouse_pos_app_space.x);
	Input::y = static_cast<int>(mouse_pos_app_space.y);

	ImGui::Image(_viewport_descriptor_sets[frame], viewport_size);

	// Gizmos
	auto cam = _editor_layer.EditorCamera();
	if(cam && cam.HasComponent<Camera>() && cam.GetComponent<Camera>().IsActive())
	{
		// TODO: Check if selected is Screen or World Space Entity
		auto e = _editor_layer.SelectedEntity();
		if(e)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			float window_width = (float)(ImGui::GetWindowWidth());
			float window_height = (float)(ImGui::GetWindowHeight());
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, window_width, window_height);

			// Camera
			const Camera& camera = cam.GetComponent<Camera>();
			const glm::mat4& view = camera.GetView();

			glm::mat4 proj = camera.GetProj();
			proj[1][1] *= -1;

			// Entity transform
			Transform& transform = e.GetComponent<Transform>();
			glm::mat4 transform_matrix = transform.model_matrix;

			ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, glm::value_ptr(transform_matrix));

			glm::vec3 translation, rotation, scale = {};

			DecomposeTransform(transform_matrix, translation, rotation, scale);

			if(ImGuizmo::IsUsing())
			{
				transform.position = translation;
				transform.scale = scale;

				// TODO: Fix rotations 
				// glm::vec3 delta_rotation = rotation - transform.rotation;
				// transform.rotation += delta_rotation;
			}
		}
	}

	ImGui::End();
}
