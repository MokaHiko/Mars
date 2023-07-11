#ifndef IRENDERPIPELINE_H
#define IRENDERPIPELINE_H

#pragma once

#include "Renderer/Renderer.h"
#include "Renderer/Vulkan/VulkanUtils.h"

#include "ECS/Scene.h"
#include "ECS/Entity.h"

namespace mrs
{
    //A render pipeline performs a series of operations that take the contents of a Scene, and displays them on a screen
    //A single render pipeline may have one or more pipelines.
    class IRenderPipeline
    {
    public:
        IRenderPipeline() {};
        virtual ~IRenderPipeline() {};

        virtual void Init() {};

        virtual void Compute(VkCommandBuffer cmd, uint32_t current_frame, float dt) {};

        // Called at start of render pass 
        virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame) {};

        // Called at end of renderpass before command buffer submit and renderpass end
        virtual void End(VkCommandBuffer cmd) {};

        // Called after command buffer recording and before main render pass (where you should place your own renderpasses)
        virtual void OnPreRenderPass(VkCommandBuffer cmd) {};

        // Called after main render pass
        virtual void OnPostRenderpass(VkCommandBuffer cmd) {};

        // Called on renderable destroyed
        virtual void OnEntityDestroyed(Entity e) {};
    protected:
        friend class IRenderPipelineLayer;

        // Scene handle
        Scene *_scene = nullptr;
        Window *_window = nullptr;

        // Base resources
        Renderer *_renderer = nullptr;
        VulkanDevice *_device = nullptr;

        // Main Render pass
        VkRenderPass _default_render_pass = VK_NULL_HANDLE;

        // Shared Global Descriptor sets
        VkDescriptorSetLayout _global_descriptor_set_layout = VK_NULL_HANDLE;
        VkDescriptorSetLayout _object_descriptor_set_layout = VK_NULL_HANDLE;
        VkDescriptorSetLayout _default_image_set_layout = VK_NULL_HANDLE;

        VkDescriptorSet _global_descriptor_set = VK_NULL_HANDLE;
    };
}

#endif