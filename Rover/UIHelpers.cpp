#include "UIHelpers.h"

void mrs::HelpMarker(const char *desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::BeginTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void mrs::TextCentered(std::string text)
{
	auto window_width = ImGui::GetWindowSize().x;
	auto text_width = ImGui::CalcTextSize(text.c_str()).x;

	ImGui::SetCursorPosX((window_width - text_width) * 0.5f);
	ImGui::Text(text.c_str());
}

bool mrs::ButtonCentered(std::string text)
{
	auto window_width = ImGui::GetWindowSize().x;
	auto button_width = ImGui::CalcTextSize(text.c_str()).x;

	ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
	return ImGui::Button(text.c_str());
}

bool mrs::ButtonBottom(std::string text)
{
	auto window_width = ImGui::GetWindowSize().x;
	auto window_height = ImGui::GetWindowSize().y;

	auto button_height = ImGui::CalcTextSize(text.c_str()).y;

	ImGui::SetCursorPosY((window_height - button_height) * 0.75f);

	return ImGui::Button(text.c_str(), ImVec2{window_width, 0});
}
