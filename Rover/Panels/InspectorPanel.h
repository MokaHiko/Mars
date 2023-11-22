#ifndef INSPECTORPANEL_H
#define INSPECTORPANEL_H

#pragma once

#include <imgui.h>

#include <ECS/Components/Components.h>
#include "Components/ComponentPanels.h"

//#include <imgui/backends/imgui_stdlib.h>
namespace mrs {
    class InspectorPanel
    {
    public:
        static void Draw(Entity entity);

        template<typename T>
        static void AddCustomComponentInspector()
		{
			_custom_component_fns.push_back(DrawComponent<T>);
		}
    private:
        static void DrawAddComponent(Entity entity);
        static std::vector<std::function<void(Entity)>> _custom_component_fns;
    };
}
#endif