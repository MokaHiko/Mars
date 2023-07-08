#ifndef MAINMENU_H
#define MAINMENU_H

#pragma once

#include <Core/Application.h>
#include <ECS/SceneSerializer.h>
#include "UIHelpers.h"

namespace mrs
{
    class MainMenu
    {
    public:
        static void Draw(Scene* scene)
        {
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Save Scene"))
                    {
                        SceneSerializer serializer(scene);
                        serializer.SerializeText(Application::GetInstance().GetAppName(), "Assets/Scenes");
                    }

                    if (ImGui::MenuItem("Load Scene"))
                    {
                        SceneSerializer serializer(scene);
                        std::string scene_path = "Assets/Scenes/" + Application::GetInstance().GetAppName() + ".yaml";
                        serializer.DeserializeText(scene_path);
                    }

                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit"))
                {
                    if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                    if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                    ImGui::Separator();
                    if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                    if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                    if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
        }
    private:

    };
}


#endif