#ifndef INSPECTORPANEL_H
#define INSPECTORPANEL_H

#pragma once

#include <imgui.h>

#include <ECS/Components/Components.h>
#include "Components/ComponentPanels.h"

//#include <imgui/backends/imgui_stdlib.h>

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
                Tag& tag = entity.GetComponent<Tag>();

                //ImGui::InputText("##label", &tag.tag);

                ImGui::Separator();

                DrawComponent<Transform>(entity);
                DrawComponent<RenderableObject>(entity);
                DrawComponent<Camera>(entity);
                DrawComponent<Script>(entity);  
                DrawComponent<RigidBody2D>(entity);  
                DrawComponent<ParticleSystem>(entity);
                DrawComponent<DirectionalLight>(entity);

                #ifdef MRS_DEBUG
                DrawComponent<Serializer>(entity);
                #endif
                
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
                static std::vector<const char*> components = [&]() {
                    std::vector<const char*> comp = { "Transform", "Renderable", "Camera", "RigidBody2D", "ParticleSystem"};
                    for(auto it = Script::script_instantiation_bindings.begin(); it != Script::script_instantiation_bindings.end(); it++)
                    {
                        comp.push_back(it->first.c_str());
                    }
                    return comp;
                }();

                for (int i = 0; i < components.size(); i++)
                {
                    if (filter.PassFilter(components[i]))
                    {
                        if (ImGui::Selectable(components[i]))
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
                            default:
                                {
                                    auto& script = entity.AddComponent<Script>();
                                    script.Bind(components[i]);
                                    script.enabled = false;
                                } break;
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