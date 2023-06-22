#ifndef DEFAULTRENDERPIPELINELAYER_H
#define DEFAULTRENDERPIPELINELAYER_H

#pragma once

#include "Renderer/RenderPipelineLayers/IRenderPipelineLayer.h"

#include "Renderer/RenderPipelineLayers/RenderPipelines/MeshRenderPipeline.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/ImGuiRenderPipeline.h"

namespace mrs
{
    class DefaultRenderPipelineLayer : public IRenderPipelineLayer
    {
    public:
        DefaultRenderPipelineLayer()
        {
            PushRenderPipeline(new MeshRenderPipeline());
            PushRenderPipeline(new ImGuiRenderPipeline());
        };
        ~DefaultRenderPipelineLayer(){};
    };
}
#endif