#ifndef COMPONENTPANELS_H
#define COMPONENTPANELS_H

#pragma once

#include <ECS/Entity.h>

#include "UIHelpers.h"

namespace mrs
{
    template<typename T>
    void DrawComponent(Entity entity);

    template<typename T>
    static void DrawComponentUI(const std::string &name, Entity entity, std::function<void(T &)> ui_function)
    {
        if (!entity.HasComponent<T>())
        {
            return;
        }

        if (ImGui::CollapsingHeader(name.c_str()))
        {
            if(ImGui::IsItemHovered() && ImGui::GetIO().MouseClicked[1])
            {
                ImGui::OpenPopup("Component Options");
            }

            if(ImGui::BeginPopup("Component Options"))
            {
                bool removed = false;
                if(ImGui::Button("Remove Component"))
                {
                    removed = entity.RemoveComponent<T>();
                }
                ImGui::EndPopup();

                if(removed)
                {
                    return;
                }
            }

            ui_function(entity.GetComponent<T>());
        }
    }
};

#endif