#ifndef TRAILRENDERPIPELINE_H
#define TRAILRENDERPIPELINE_H

#pragma once

#include "Renderer/RenderPipelineLayers/IRenderPipeline.h"

namespace mrs
{
    class TrailRenderPipeline : public IRenderPipeline
    {
    public:
        TrailRenderPipeline();
        virtual ~TrailRenderPipeline();

        virtual void OnPreRenderPass(VkCommandBuffer cmd, RenderableBatch* batch);
        virtual void Init() override;
        virtual void InitDescriptors() override;

        virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch) override;
        virtual void End(VkCommandBuffer cmd) override;

        virtual void UpdateDescriptors(uint32_t current_frame, float dt, RenderableBatch* batch) override;
    private:
        void OnTrailRendererCreated(entt::basic_registry<entt::entity>&, entt::entity entity);
        void OnTrailRendererDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity);
    private:
        // Line Renderer
        std::vector<VkDescriptorSet> _global_data_sets = {};
        VkDescriptorSetLayout _global_data_set_layout = VK_NULL_HANDLE;

        VkDescriptorSetLayout _object_set_layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> _object_sets = {};

        std::vector<AllocatedBuffer> _trails_vertex_buffer = {};
        int n_trails = 0;
        int _insert_index = 0;

        Scene* _scene = nullptr;
    };
}

#endif