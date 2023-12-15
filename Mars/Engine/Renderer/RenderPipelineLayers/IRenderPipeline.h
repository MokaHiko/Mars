#ifndef IRENDERPIPELINE_H
#define IRENDERPIPELINE_H

#pragma once

#include "Renderer/Renderer.h"
#include "Renderer/Vulkan/VulkanUtils.h"
#include "Renderer/Vulkan/VulkanAssetManager.h"

#include "ECS/Scene.h"
#include "ECS/Entity.h"

#include "Math/Math.h"

namespace mrs
{
    struct ShaderEffect;
    struct RenderableBatch;

    // Indirect Drawing
    struct IndirectBatch
    {
        mrs::Mesh* mesh; 
        mrs::Material* material;

        uint32_t first; // batches first instance in draw indirect buffer
        uint32_t count; // batch member count
    };

    //A render pipeline performs a series of operations that take the contents of a Scene, and displays them on a screen. It implements an instance of a ShaderEffect.
    //A single render pipeline may have one or more actual pipelines.
    class IRenderPipeline
    {
    public:
        struct RenderPipelineSettings
        {
            VkPrimitiveTopology primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            VkPolygonMode polygon_mode = VK_POLYGON_MODE_FILL;

            std::vector<VkPushConstantRange> push_constants = {};

            // Blending
            VkBool32 blend_enable = false;
            VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            VkBlendOp colorBlendOp = VK_BLEND_OP_ZERO_EXT;
            VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            VkBlendOp alphaBlendOp = VK_BLEND_OP_ZERO_EXT;

            // When greater than 0, tesselation will be enabled
            uint32_t tesselation_control_points = 0;
        };

        IRenderPipeline(const std::string& name);
        IRenderPipeline(const std::string& name, VkRenderPass render_pass);

        virtual ~IRenderPipeline() {};

        const std::string& Name() const { return _name; }

        // Returns shader effect of render pipeline
        Ref<ShaderEffect> Effect();
    public:
        virtual void Init() {};

        virtual void InitDescriptors() {};

        virtual void UpdateDescriptors(uint32_t current_frame, float dt, RenderableBatch* batch) {};

        // Called in in compute pass
        virtual void Compute(VkCommandBuffer cmd, uint32_t current_frame, float dt, RenderableBatch* compute_batch) {};

        // Called at start of render pass 
        virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch) {};

        // Called at end of renderpass before command buffer submit and renderpass end
        virtual void End(VkCommandBuffer cmd) {};

        // Called after command buffer recording and before main render pass (where you should place your own renderpasses)
        virtual void OnPreRenderPass(VkCommandBuffer cmd, RenderableBatch* batch) {};

        // Called at the start of swapchain frame buffer render pass
        virtual void OnMainPassBegin(VkCommandBuffer cmd) {};

        // Called at the end of swapchain frame buffer render pass
        virtual void OnMainPassEnd(VkCommandBuffer cmd) {};

        virtual void OnPostMainPass(VkCommandBuffer cmd) {};

        // Called when a material's properties are changed or entity material changed
        virtual void OnMaterialsUpdate() {};

        virtual void OnViewPortResize(Vector2 dimensions) {};
    protected:
        friend class IRenderPipelineLayer;
        std::string _name;

        Window* _window = nullptr;

        Renderer* _renderer = nullptr;
        VulkanDevice* _device = nullptr;

        VkRenderPass _render_pass = VK_NULL_HANDLE;
    protected:
        RenderPipelineSettings _render_pipeline_settings = {};

        // Pushes a shader on the pipeline
        void PushShader(Ref<Shader> shader);
        void BuildPipeline();

        VkPipeline _pipeline = VK_NULL_HANDLE;
        VkPipelineLayout _pipeline_layout = VK_NULL_HANDLE;

        VkDescriptorSet _descriptor_set;
        VkDescriptorSetLayout _descriptor_set_layout;

        std::vector<VkDescriptorSet> _descriptor_sets;

        std::vector<Ref<Shader>> _shaders;
        std::vector<VkDescriptorSetLayout> _descriptor_set_layouts;
        std::vector<VulkanDescriptorSet> _required_descriptors;
    private:
        // The effect the pipeline implements
        Ref<ShaderEffect> _shader_effect = nullptr;
        void ParseDescriptorSetFromSpirV(const void* spirv_code, size_t spirv_nbytes, VkShaderStageFlagBits stage);
    };
}

#endif