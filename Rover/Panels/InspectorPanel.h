#ifndef INSPECTORPANEL_H
#define INSPECTORPANEL_H

#pragma once

#include <imgui.h>

#include <ECS/Components/Components.h>
#include "Components/ComponentPanels.h"

namespace mrs
{
    class InspectorPanel
    {
    public:
        static void Draw(Entity entity)
        {
            ImGui::Begin("Inspector");
            if (entity.Id() != entt::null)
            {
                std::string header = "Name: " + entity.GetComponent<Tag>().tag + " | Entity ID: " + std::to_string(entity.Id());
                ImGui::SeparatorText(header.c_str());

                DrawComponent<Transform>(entity);
                DrawComponent<RenderableObject>(entity);
                DrawComponent<Camera>(entity);
                DrawComponent<Script>(entity);  
                DrawComponent<RigidBody2D>(entity);  
                DrawComponent<ParticleSystem>(entity);
                DrawComponent<DirectionalLight>(entity);
                
                DrawAddComponent(entity);
            }

            ImGui::End();
        }
    private:
        static void DrawAddComponent(Entity entity)
        {
            static bool searching = false;
            ImGui::Separator();
            if (ImGui::Selectable("+ Add Component"))
            {
                searching = true;
            }

            if (searching)
            {
                static ImGuiTextFilter filter;
                filter.Draw();
                const char *lines[] = { "Transform", "Renderable", "Camera", "RigidBody2D", "ParticleSystem" };
                for (int i = 0; i < IM_ARRAYSIZE(lines); i++)
                {
                    if (filter.PassFilter(lines[i]))
                    {
                        if (ImGui::Selectable(lines[i]))
                        {
                            switch (i)
                            {
                            case 0:
                                entity.AddComponent<Transform>();
                                break;
                            case 1:
                                entity.AddComponent<RenderableObject>();
                                break;
                            case 2:
                                entity.AddComponent<Camera>();
                                break;
                            case 3:
                                entity.AddComponent<RigidBody2D>();
                                break;
                            case 4:
                                entity.AddComponent<ParticleSystem>();
                                break;
                            }
                            searching = false;
                        }
                    }
                }
            }
        }
    };
}
#endif