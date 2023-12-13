#ifndef SPRITERENDERPIPELINE_H
#define SPRITERENDERPIPELINE_H

#pragma once

#include "Renderer/RenderPipelineLayers/IRenderPipeline.h"
#include "Renderer/Textures/Sprite.h"

namespace mrs
{
    class SpriteRenderPipeline : public IRenderPipeline
    {
    public:
        SpriteRenderPipeline();
        virtual ~SpriteRenderPipeline();

        virtual void Init() override;
        virtual void InitDescriptors() override;

        virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch) override;
        virtual void End(VkCommandBuffer cmd) override;
    private:
        void UpdateDescriptors();
    private:
        // TODO: Make Sprite Push constant data and put in storage buffer
        struct SpritePushConstant 
        { 
            Rectangle atlas_rect;
            Rectangle sprite_rect;
        };

        // Mesh
        std::vector<VkDescriptorSet> _global_data_sets = {};
        VkDescriptorSetLayout _global_data_set_layout = VK_NULL_HANDLE;

        VkDescriptorSetLayout _object_set_layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> _object_sets = {};
    private:
        float viewport_width, viewport_height;
    };
}

#endif