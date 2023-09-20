#ifndef POSTPROCESSINGRENDERPIPELINE_H
#define POSTPROCESSINGRENDERPIPELINE_H

#pragma once

#include "Renderer/RenderPipelineLayers/IRenderPipeline.h"

namespace mrs
{
    class PostProcessingRenderPipeline : public IRenderPipeline
    {
    public:
        PostProcessingRenderPipeline();
        virtual ~PostProcessingRenderPipeline();

        virtual void Init() override;

        virtual void OnMainPassBegin(VkCommandBuffer cmd) override;
    private:
        void InitPostProcessPipeline();
        void InitDescriptors();
    private:
        std::vector<VkDescriptorSet> _post_process_descriptor_sets;
        VkSampler _screen_sampler;

        VkDescriptorSetLayout _post_process_descriptor_set_layout;

        VkPipelineLayout _post_process_pipeline_layout;
        VkPipeline _post_process_pipeline;
    };
}

#endif