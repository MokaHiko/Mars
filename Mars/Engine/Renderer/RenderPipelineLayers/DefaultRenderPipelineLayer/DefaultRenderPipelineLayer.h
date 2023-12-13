#ifndef DEFAULTRENDERPIPELINELAYER_H
#define DEFAULTRENDERPIPELINELAYER_H

#pragma once

#include "Renderer/RenderPipelineLayers/IRenderPipelineLayer.h"

#include "Renderer/RenderPipelineLayers/RenderPipelines/MeshRenderPipeline.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/ParticleRenderPipeline/ParticleRenderPipeline.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/ImGuiRenderPipeline/ImGuiRenderPipeline.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/PostProcessingRenderPipeline/PostProcessingRenderPipeline.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/TerrainRenderPipeline/TerrainRenderPipeline.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/SpriteRenderPipeline/SpriteRenderPipeline.h"

namespace mrs
{
    class DefaultRenderPipelineLayer : public IRenderPipelineLayer
    {
    public:
        DefaultRenderPipelineLayer();
        ~DefaultRenderPipelineLayer() {};
    };
}
#endif