#ifndef PERFORMANCE_PANEL_H
#define PERFORMANCE_PANEL_H

#pragma once

#include <Core/Memory.h>
#include "IPanel.h"

namespace mrs {
    class Renderer;
    class IRenderPipelineLayer;
    class PerformancePanel : public IPanel
    {
    public:
        PerformancePanel(EditorLayer* editor_layer, const std::string& name, IRenderPipelineLayer* render_pipeline_layer);
        virtual ~PerformancePanel();

        virtual void Draw() override;
    private:
        IRenderPipelineLayer* _render_pipeline_layer;
        Ref<Renderer> _renderer;
    };
}

#endif