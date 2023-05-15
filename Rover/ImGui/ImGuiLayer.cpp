#include "ImGuiLayer.h"
#include <imgui_impl_vulkan.h>

#include "Core/Application.h"

void mrs::ImGuiLayer::OnAttach()
{
	_window_handle = (SDL_Window*)Application::GetInstance().GetWindow()->GetNativeWindow();
}

void mrs::ImGuiLayer::OnUpdate(float dt)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame(_window_handle);

	ImGui::NewFrame();
}
