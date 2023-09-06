#include "HierarchyPanel.h"

#include <ECS/Components/Components.h>
#include <imgui.h>

#include "InspectorPanel.h"
#include "Rover.h"

mrs::HierarchyPanel::HierarchyPanel(EditorLayer *editor_layer,const std::string &name, Scene *scene)
	:IPanel(editor_layer, name), _scene(scene) {}

mrs::HierarchyPanel::~HierarchyPanel() {}

void mrs::HierarchyPanel::Draw()
{
	auto view = _scene->Registry()->view<Tag, Transform>();
	ImGui::Begin("Entity Hierarchy");
	int ctr = 0;

	// Select entity
	for (auto entity : view)
	{
		ImGui::PushID(ctr++);

		Entity e = { entity, _scene };
		Tag& tag = e.GetComponent<Tag>();

		if (ImGui::Selectable(tag.tag.c_str(), e == _editor_layer->SelectedEntity()))
		{
			if (ImGui::IsMouseClicked(1))
			{
				MRS_INFO("Object options!");
				ImGui::OpenPopup("Object Options");
			}

			if (ImGui::BeginPopup("Object Options"))
			{
				ImGui::Button("Delete Object");
				ImGui::EndPopup();
			}

			_editor_layer->FocusEntity(e);
		}
		ImGui::PopID();
	}

	InspectorPanel::Draw(_editor_layer->SelectedEntity());

	// Creating entity
	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow))
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("Create Options");
		}
	}

	if (ImGui::BeginPopup("Create Options"))
	{
		if (ImGui::Button("New Entity"))
		{
			_scene->Instantiate("New Entity");
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::End();
}
