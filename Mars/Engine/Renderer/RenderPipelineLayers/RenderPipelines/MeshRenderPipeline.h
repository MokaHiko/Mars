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

        virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch) override;
        virtual void End(VkCommandBuffer cmd) override;

        virtual void OnPreRenderPass(VkCommandBuffer cmd, RenderableBatch* batch) override;
        virtual void OnMaterialsUpdate() override;

        virtual void OnRenderableCreated(Entity e) override;
        virtual void OnRenderableDestroyed(Entity e) override;
    private:
        void CreateOffScreenFramebuffer();

        void InitIndirectCommands();
        void InitOffScreenPipeline();

        void BuildBatches(VkCommandBuffer cmd, RenderableBatch* scene);
        void RecordIndirectcommands(VkCommandBuffer cmd, RenderableBatch* scene);

        void DrawShadowMap(VkCommandBuffer cmd, RenderableBatch *batch);
        void DrawObjects(VkCommandBuffer cmd, RenderableBatch *batch);
    private:
        // Mesh
        VkDescriptorSet _global_data_set = VK_NULL_HANDLE;
        VkDescriptorSetLayout _global_data_set_layout = VK_NULL_HANDLE;

        VkDescriptorSetLayout _object_set_layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> _object_sets = {};

        VkDescriptorSetLayout _dir_light_layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>  _dir_light_sets = {};
    private:
        // Shadows
        VkFramebuffer _offscreen_framebuffer;
        AllocatedImage _offscreen_depth_image;
        VkImageView _offscreen_depth_image_view;

        VkRenderPass _offscreen_render_pass;
        VkPipeline _offscreen_render_pipeline;
        VkPipelineLayout _offscreen_render_pipeline_layout;

        VkSampler _shadow_map_sampler;
        VkDescriptorSetLayout _shadow_map_descriptor_layout;
        VkDescriptorSet _shadow_map_descriptor;
    private:
        struct IndirectBatch
        {
            Mesh *mesh;
            Material *material;

            uint32_t first; // batches first instance in draw indirect buffer
            uint32_t count; // batch member count
        };

        // Returns vector of indirect draw batches from renderable batch
        std::vector<IndirectBatch> GetRenderablesAsBatches(RenderableBatch* batch);

        // Flag set when draw commands need to be updated i.e Entity creation and destruction
        bool _rerecord = true;

        std::vector<AllocatedBuffer> _indirect_buffers;
        std::vector<IndirectBatch> _batches = {};
    };
}

#endif