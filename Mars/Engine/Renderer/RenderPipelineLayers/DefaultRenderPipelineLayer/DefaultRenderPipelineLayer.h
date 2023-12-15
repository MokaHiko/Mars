#ifndef DEFAULTRENDERPIPELINELAYER_H
#define DEFAULTRENDERPIPELINELAYER_H

#pragma once

#include "Renderer/RenderPipelineLayers/IRenderPipelineLayer.h"


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