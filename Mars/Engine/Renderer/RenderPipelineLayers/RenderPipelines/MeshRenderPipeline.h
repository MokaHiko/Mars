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

        virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch) override;
        virtual void End(VkCommandBuffer cmd) override;

        virtual void OnPreRenderPass(VkCommandBuffer cmd, RenderableBatch* batch) override;
        virtual void OnMaterialsUpdate() override;
    public:
        // Called on renderable created
        virtual void OnRenderableCreated(Entity e) override;

        // Called on renderable destroyed
        virtual void OnRenderableDestroyed(Entity e) override;
    private:
        void InitDescriptors();
        void InitPipelineLayout();

        void CreateOffScreenFramebuffer();

        void InitIndirectCommands();
        void InitMeshPipeline();
        void InitOffScreenPipeline();

        void BuildBatches(VkCommandBuffer cmd, RenderableBatch* scene);
        void RecordIndirectcommands(VkCommandBuffer cmd, RenderableBatch* scene);

        void DrawShadowMap(VkCommandBuffer cmd, RenderableBatch *scene);
        void DrawObjects(VkCommandBuffer cmd, RenderableBatch *scene);
    private:
        // Mesh
        VkDescriptorSet _frame_object_set = VK_NULL_HANDLE;
    private:
        // Shadows
        VkFramebuffer _offscreen_framebuffer;
        AllocatedImage _offscreen_depth_image;
        VkImageView _offscreen_depth_image_view;

        VkRenderPass _offscreen_render_pass;
        VkPipeline _offscreen_render_pipeline;

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

        // Returns vector if indirect batches from renderables from scene
        std::vector<IndirectBatch> GetRenderablesAsBatches(RenderableBatch* batch);

        // Flag set when draw commands need to be updated i.e Entity creation and destruction
        bool _rerecord = true;

        std::vector<AllocatedBuffer> _indirect_buffers;
        std::vector<IndirectBatch> _batches = {};
    };
}

#endif