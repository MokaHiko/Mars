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
    //A render pipeline performs a series of operations that take the contents of a Scene, and displays them on a screen
    //A single render pipeline may have one or more pipelines.
    class IRenderPipeline
    {
    public:
        IRenderPipeline(const std::string& name);
        IRenderPipeline(const std::string& name, Renderer *renderer, VkRenderPass _render_pass = nullptr, VkFormat _render_pass_format = VK_FORMAT_UNDEFINED);

        virtual ~IRenderPipeline() {};

        const std::string& Name() const {return _name;}
    public:
        virtual void Init() {}

        // Called in in compute pass
        virtual void Compute(VkCommandBuffer cmd, uint32_t current_frame, float dt) {};

        // Called at start of render pass 
        virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame) {};

        // Called at end of renderpass before command buffer submit and renderpass end
        virtual void End(VkCommandBuffer cmd) {};

        // Called after command buffer recording and before main render pass (where you should place your own renderpasses)
        virtual void OnPreRenderPass(VkCommandBuffer cmd) {};

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

        // Scene handle
        Scene *_scene = nullptr;
        Window *_window = nullptr;

        // Base resources
        Renderer *_renderer = nullptr;
        VulkanDevice *_device = nullptr;

        // Render pass
        VkRenderPass _render_pass = VK_NULL_HANDLE;
        VkFormat _render_pass_format = VK_FORMAT_UNDEFINED;

        // Global Descriptor sets
        VkDescriptorSetLayout _global_descriptor_set_layout = VK_NULL_HANDLE;
        VkDescriptorSetLayout _object_descriptor_set_layout = VK_NULL_HANDLE;
        VkDescriptorSet _global_descriptor_set = VK_NULL_HANDLE;

        // Vulkan asset manager
        VulkanAssetManager* _asset_manager = nullptr;
    };
}

#endif