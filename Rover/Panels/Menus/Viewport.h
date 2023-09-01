#ifndef VIEWPORT_H
#define VIEWPORT_H

#pragma once

#include <Renderer/Vulkan/VulkanInitializers.h>
#include <Renderer/RenderPipelineLayers/IRenderPipelineLayer.h>

namespace mrs
{
    class Viewport
    {
    public:
        static void Init(IRenderPipelineLayer* render_pipeline_layer);
        static void Draw();

    private:
        static Ref<Renderer> _renderer;
        static VkSampler _viewport_sampler;


        static std::vector<VkDescriptorSet> _viewport_descriptor_sets;
    };
}

#endif