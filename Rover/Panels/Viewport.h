#ifndef VIEWPORT_H
#define VIEWPORT_H

#pragma once

#include <Renderer/Vulkan/VulkanInitializers.h>
#include <Renderer/RenderPipelineLayers/IRenderPipelineLayer.h>

#include "IPanel.h"

namespace mrs
{
    class Viewport : public IPanel
    {
    public:
        Viewport(EditorLayer& editor_layer, const std::string& name, IRenderPipelineLayer* render_pipeline_layer);
        virtual ~Viewport();

        virtual void Draw() override;
    private:
        Ref<Renderer> _renderer;
        VkSampler _viewport_sampler;

        VkDescriptorSet _viewport_descriptor_set;
        bool _focus_state = true;
    };
}

#endif