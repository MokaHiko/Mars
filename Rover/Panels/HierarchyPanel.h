#ifndef HIERARCHYPANEL_H
#define HIERARCHYPANEL_H

#pragma once

#include "IPanel.h"

namespace mrs {
    class Scene;
    class HierarchyPanel : public IPanel
    {
    public:
        HierarchyPanel(EditorLayer* editor_layer, const std::string& name, Scene* scene);
        virtual ~HierarchyPanel();

        virtual void Draw() override;
    private:
        Scene* _scene;
    };
}

#endif