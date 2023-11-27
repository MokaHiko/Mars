#ifndef UIRENDERPIPELINE_H
#define UIRENDERPIPELINE_H

#pragma once

#include "Renderer/RenderPipelineLayers/IRenderPipeline.h"

namespace mrs
{
    class UIRenderPipeline : public IRenderPipeline
    {
    public:
        UIRenderPipeline();
        virtual ~UIRenderPipeline();

        virtual void Init() override;
        virtual void InitDescriptors() override;

        virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch) override;
        virtual void End(VkCommandBuffer cmd) override;
    private:
        // Mesh
        VkDescriptorSet _global_data_set = VK_NULL_HANDLE;
        VkDescriptorSetLayout _global_data_set_layout = VK_NULL_HANDLE;

        VkDescriptorSetLayout _object_set_layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> _object_sets = {};
    private:
        float viewport_width, viewport_height;
    };
}

#endif