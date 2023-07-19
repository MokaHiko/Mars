#ifndef SHADOWRENDERPIPELINE_H
#define SHADOWRENDERPIPELINE_H

#pragma once

#include "Renderer/RenderPipelineLayers/IRenderPipeline.h"

namespace mrs
{
    // Deffered Shadows
    class ShadowRenderPipeline : public IRenderPipeline
    {
    public:
        virtual void OnPreRenderPass(VkCommandBuffer cmd) override;

        void DrawShadowMap(VkCommandBuffer cmd, Scene *scene);
    private:
        void InitDescriptors();

        void InitPipelineLayout();
        void InitOffScreenPipeline();

        // Shadows
        VkFramebuffer _offscreen_framebuffer;
        AllocatedImage _offscreen_depth_image;
        VkImageView _offscreen_depth_image_view;

        VkRenderPass _offscreen_render_pass;
        VkPipeline _offscreen_render_pipeline;
        VkPipelineLayout _offscreen_pipeline_layout;

        VkSampler _shadow_map_sampler;
        VkDescriptorSetLayout _shadow_map_descriptor_layout;
        VkDescriptorSet _shadow_map_descriptor;
    };
}

#endif