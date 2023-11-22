#include "HierarchyPanel.h"

#include <ECS/Components/Components.h>
#include <imgui.h>

#include "InspectorPanel.h"
#include "Rover.h"

mrs::HierarchyPanel::HierarchyPanel(EditorLayer* editor_layer, const std::string& name, Scene* scene)
	:IPanel(editor_layer, name), _scene(scene) {}

mrs::HierarchyPanel::~HierarchyPanel() {}

void mrs::HierarchyPanel::Draw()
{
	ImGui::Begin("Entity Hierarchy");

	// Recursively draw scene hierarchy
	Entity root = _scene->Root();
	DrawNodeRecursive(root);

	// Inspect selected entity
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

void mrs::HierarchyPanel::DrawNodeRecursive(Entity node)
{
	static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
	ImGuiTreeNodeFlags node_flags = base_flags;

	const Transform& node_transform = node.GetComponent<Transform>();
	const Tag& node_tag = node.GetComponent<Tag>();

	bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)(node.Id()), node_flags, "%s", node_tag.tag.c_str());

	if (ImGui::IsItemClicked())
	{
		_editor_layer->FocusEntity(node);
	}

	if (_editor_layer->SelectedEntity() == node)
	{
		node_flags |= ImGuiTreeNodeFlags_Selected;
	}

	if (node_open)
	{
		for (uint32_t i = 0; i < node_transform.children_count; i++)
		{
			Entity e = { node_transform.children[i], _scene };
			DrawNodeRecursive(node_transform.children[i]);
		}

		ImGui::TreePop();
	}
}
