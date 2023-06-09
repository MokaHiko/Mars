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
        MeshRenderPipeline() {};
        ~MeshRenderPipeline() {};

        virtual void Init() override;

        virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame) override;
        virtual void End(VkCommandBuffer cmd) override;

        virtual void OnPreRenderPass(VkCommandBuffer cmd) override;
    private:
        void InitDescriptors();
        void InitPipelineLayout();
        void InitPipelines();

        void CreateOffScreenFramebuffer();

        void InitIndirectCommands();
        void InitMeshPipeline();
        void InitOffScreenPipeline();

        void DrawShadowMap(VkCommandBuffer cmd, Scene *scene);
        void DrawObjects(VkCommandBuffer cmd, Scene *scene);
    private:
        // Mesh
        VkPipeline _default_pipeline;
        VkPipelineLayout _default_pipeline_layout;

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
        // Indirect Draws
        struct IndirectBatch
        {
            Mesh *mesh;
            Material *material;

            uint32_t first; // batches first instance in draw indirect buffer
            uint32_t count; // batch member count
        };

        // Returns vector if indirect batches from renderables from scene
        std::vector<IndirectBatch> GetRenderablesAsBatches(Scene *scene);

        // Buffers for indirect render commands
        std::vector<AllocatedBuffer> _indirect_buffers;
    };
}

#endif