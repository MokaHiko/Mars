#ifndef DEFAULTRENDERPIPELINELAYER_H
#define DEFAULTRENDERPIPELINELAYER_H

#pragma once

#include "Renderer/RenderPipelineLayers/IRenderPipelineLayer.h"

#include "Renderer/RenderPipelineLayers/RenderPipelines/ParticleRenderPipeline/ParticleRenderPipeline.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/MeshRenderPipeline.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/ImGuiRenderPipeline.h"

namespace mrs
{
    class DefaultRenderPipelineLayer : public IRenderPipelineLayer
    {
    public:
        DefaultRenderPipelineLayer()
        {
            PushRenderPipeline(new ImGuiRenderPipeline());
            PushRenderPipeline(new ParticleRenderPipeline(10000));
            PushRenderPipeline(new MeshRenderPipeline());
        };
        ~DefaultRenderPipelineLayer() {};
    };
}
#endif