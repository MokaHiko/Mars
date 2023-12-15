#ifndef MESHRENDERPIPELINE_H
#define MESHRENDERPIPELINE_H

#pragma once

#include "Renderer/RenderPipelineLayers/IRenderPipeline.h"

namespace mrs
{
    // Default mesh render pipeline
    class MeshRenderPipeline : public IRenderPipeline
    {
    public:
        MeshRenderPipeline();
        ~MeshRenderPipeline();

        virtual void Init() override;
        virtual void InitDescriptors() override;

        virtual void UpdateDescriptors(uint32_t current_frame, float dt, RenderableBatch* batch) override;

        virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch) override;
        virtual void End(VkCommandBuffer cmd) override;

        virtual void OnPreRenderPass(VkCommandBuffer cmd, RenderableBatch* batch) override;
        virtual void OnMaterialsUpdate() override;

        void OnMeshRendererCreated(entt::basic_registry<entt::entity>&, entt::entity entity);
        void OnMeshRendererDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity);
    private:
        void CreateOffScreenFramebuffer();

        void InitIndirectCommands();
        void InitOffScreenPipeline();

        void BuildBatches(RenderableBatch* scene);
        void RecordIndirectcommands(uint32_t current_frame, RenderableBatch* batch);

        void DrawShadowMap(VkCommandBuffer cmd, RenderableBatch *batch);
        void DrawObjects(VkCommandBuffer cmd, uint32_t current_frame);
    private:
        // Mesh
        std::vector<VkDescriptorSet> _global_data_sets = {};
        VkDescriptorSetLayout _global_data_set_layout = VK_NULL_HANDLE;

        VkDescriptorSetLayout _object_set_layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> _object_sets = {};

        VkDescriptorSetLayout _dir_light_layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>  _dir_light_sets = {};
    private:
        // Shadows
        VkFramebuffer _offscreen_framebuffer = VK_NULL_HANDLE;
        AllocatedImage _offscreen_depth_image = {};
        VkImageView _offscreen_depth_image_view = VK_NULL_HANDLE;

        VkRenderPass _offscreen_render_pass = VK_NULL_HANDLE;
        VkPipeline _offscreen_render_pipeline = VK_NULL_HANDLE;
        VkPipelineLayout _offscreen_render_pipeline_layout = VK_NULL_HANDLE;

        VkSampler _shadow_map_sampler = VK_NULL_HANDLE;
        VkDescriptorSetLayout _shadow_map_descriptor_layout = VK_NULL_HANDLE;
        VkDescriptorSet _shadow_map_descriptor = VK_NULL_HANDLE;
    private:
        // Returns vector of indirect draw batches from renderable batch
        std::vector<IndirectBatch> GetRenderablesAsBatches(RenderableBatch* batch);

        // Flag set when draw commands need to be updated i.e Entity creation and destruction
        bool _rerecord = true;

        std::vector<AllocatedBuffer> _indirect_buffers = {};
        std::vector<IndirectBatch> _batches = {};
    };
}

#endif