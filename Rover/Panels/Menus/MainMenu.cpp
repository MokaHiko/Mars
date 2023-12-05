#include "MainMenu.h"

#include <Core/Application.h>
#include <ECS/SceneSerializer.h>

#include "UIHelpers.h"
#include "Rover.h"

mrs::MainMenu::MainMenu(EditorLayer& editor_layer, const std::string &name,Scene *scene)
	: IPanel(editor_layer, name), _scene(scene) {}

mrs::MainMenu::~MainMenu() {}

void mrs::MainMenu::Draw() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Scene")) {
      if (ImGui::MenuItem("Save Scene")) {
        SceneSerializer serializer(_scene);
        serializer.SerializeText(Application::Instance().GetAppName(),
                                 "Assets/Scenes");
      }

      if (ImGui::MenuItem("Load Scene")) {
        SceneSerializer serializer(_scene);
        std::string scene_path = "Assets/Scenes/" +
                                 Application::Instance().GetAppName() +
                                 ".yaml";
        serializer.DeserializeText(scene_path);
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
      if (ImGui::MenuItem("Undo", "CTRL+Z")) {
      }
      if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {
      } // Disabled item
      ImGui::Separator();
      if (ImGui::MenuItem("Cut", "CTRL+X")) {
      }
      if (ImGui::MenuItem("Copy", "CTRL+C")) {
      }
      if (ImGui::MenuItem("Paste", "CTRL+V")) {
      }
      ImGui::EndMenu();
    }

	std::string state = _editor_layer.State() == EditorLayer::EditorState::Playing ? "Pause" : "Play";
	if (ImGui::Button(state.c_str()))
	{
		if (_editor_layer.State() == EditorLayer::EditorState::Playing)
		{
			_editor_layer.Pause();
		}
		else
		{
			_editor_layer.Play();
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Stop"))
	{
		_editor_layer.Stop();
	}

    ImGui::EndMainMenuBar();
  }
}
