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
    private:
        static void DrawAddComponent(Entity entity);
    };
}
#endif