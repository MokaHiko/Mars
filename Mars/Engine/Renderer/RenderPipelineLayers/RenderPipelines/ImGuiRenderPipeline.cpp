#include "ImGuiRenderPipeline.h"

#include <imgui_impl_vulkan.h>

#include "Core/Application.h"
#include "Renderer/Vulkan/VulkanUtils.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"

void mrs::ImGuiRenderPipeline::Init() 
{
	_window_handle = (SDL_Window *)Application::GetInstance().GetWindow()->GetNativeWindow();

	// 1: create descriptor pool for IMGUI
	//  the size of the pool is very oversize, but it's copied from imgui demo itself.
	VkDescriptorPoolSize pool_sizes[] =
		{
			{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
			{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
			{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
			{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
			{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
			{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	VkDescriptorPool imguiPool;
	VK_CHECK(vkCreateDescriptorPool(_renderer->GetDevice().device, &pool_info, nullptr, &imguiPool));

	// 2: initialize imgui library

	// this initializes the core structures of imgui
	ImGui::CreateContext();

	// this initializes imgui for SDL
	ImGui_ImplSDL2_InitForVulkan((SDL_Window *)Application::GetInstance().GetWindow()->GetNativeWindow());

	// this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = _renderer->GetInstance();
	init_info.PhysicalDevice = _renderer->GetDevice().physical_device;
	init_info.Device = _renderer->GetDevice().device;
	init_info.Queue = _renderer->GetQueues().graphics;
	init_info.DescriptorPool = imguiPool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, _renderer->GetRenderPass());

	// execute a gpu command to upload imgui font textures
	_renderer->ImmediateSubmit([&](VkCommandBuffer cmd)
							  { ImGui_ImplVulkan_CreateFontsTexture(cmd); });

	// clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();

	// add the destroy the imgui created structures
	_renderer->GetDeletionQueue().Push([=]()
									  {
		vkDestroyDescriptorPool(_renderer->GetDevice().device, imguiPool, nullptr);
		ImGui_ImplVulkan_Shutdown(); });
}

void mrs::ImGuiRenderPipeline::Begin(VkCommandBuffer cmd, uint32_t current_frame) 
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame(_window_handle);
	ImGui::NewFrame();
}

void mrs::ImGuiRenderPipeline::End(VkCommandBuffer cmd) 
{
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}
