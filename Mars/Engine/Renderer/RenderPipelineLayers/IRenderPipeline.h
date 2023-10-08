#ifndef IRENDERPIPELINE_H
#define IRENDERPIPELINE_H

#pragma once

#include "Renderer/Renderer.h"
#include "Renderer/Vulkan/VulkanUtils.h"
#include "Renderer/Vulkan/VulkanAssetManager.h"

#include "ECS/Scene.h"
#include "ECS/Entity.h"


namespace mrs
{
    struct ShaderEffect;
    struct RenderableBatch;

    struct VulkanDescriptorSet
    {
        uint32_t set = -1;
        std::unordered_map<uint32_t, VkDescriptorType> bindings;
        VkShaderStageFlagBits shader_stage;
        VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;

        void AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits stage)
        {
            bindings[binding] = type;
            shader_stage = static_cast<VkShaderStageFlagBits>(shader_stage | stage);
        }
    };

    //A render pipeline performs a series of operations that take the contents of a Scene, and displays them on a screen. It implements an instance of a ShaderEffect.
    //A single render pipeline may have one or more actual pipelines.
    class IRenderPipeline
    {
    public:
        IRenderPipeline(const std::string& name);
        IRenderPipeline(const std::string& name, VkRenderPass render_pass);

        virtual ~IRenderPipeline() {};

        const std::string& Name() const {return _name;}

        Ref<ShaderEffect> Effect();
    public:
        virtual void Init() {};

        virtual void InitDescriptors() {};

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

        // Called on renderable created
        virtual void OnRenderableCreated(Entity e) {};

        // Called on renderable destroyed
        virtual void OnRenderableDestroyed(Entity e) {};

        // Called when a material's properties are changed or entity material changed
        virtual void OnMaterialsUpdate() {};
    protected:
        friend class IRenderPipelineLayer;
        std::string _name;

        Window *_window = nullptr;

        Renderer *_renderer = nullptr;
        VulkanDevice *_device = nullptr;

        VkRenderPass _render_pass = VK_NULL_HANDLE;
    protected:
        // Pushes a shader on the pipeline
        void PushShader(Ref<Shader> shader);
        void BuildPipeline();

        VkPipeline _pipeline = VK_NULL_HANDLE;
        VkPipelineLayout _pipeline_layout = VK_NULL_HANDLE;

        VkDescriptorSet _descriptor_set;
        VkDescriptorSetLayout _descriptor_set_layout;

        std::vector<VkDescriptorSet> _descriptor_sets;

        // The effect this pipeline implements
        Ref<ShaderEffect> _shader_effect = nullptr;

        std::vector<Ref<Shader>> _shaders;
        std::vector<VkDescriptorSetLayout> _descriptor_set_layouts;
        std::vector<VulkanDescriptorSet> _required_descriptors;
    private:
        void ParseDescriptorSetFromSpirV(const void* spirv_code, size_t spirv_nbytes, VkShaderStageFlagBits stage);
    };
}

#endif