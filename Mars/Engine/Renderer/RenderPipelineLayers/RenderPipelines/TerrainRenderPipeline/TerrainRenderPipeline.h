#ifndef TERRAINRENDERPIPELINE_H
#define TERRAINRENDERPIPELINE_H

#pragma once

#include <glm/glm.hpp>
#include "Renderer/Vulkan/VulkanUtils.h"
#include "Renderer/RenderPipelineLayers/IRenderPipeline.h"

namespace mrs
{
    struct TerrainGraphicsPushConstant 
    {
        uint32_t material_index;
    };

    class TerrainRenderPipeline : public IRenderPipeline
    {
    public:
        virtual void Init();

        virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame) ;

        virtual void End(VkCommandBuffer cmd) ;
    private:
        void CreateTerrainPipelineLayout();
        void CreateTerrainPipeline();

        VkPipeline _terrain_render_pipeline;
        VkPipelineLayout _terrain_render_pipeline_layout;

        VkDescriptorSet _terrain_descriptor_set;
        VkDescriptorSetLayout _terrain_descriptor_set_layout;

        AllocatedBuffer _terrain_descriptor_set_uniform_buffer;

        Texture _terrain_height_map;
        std::shared_ptr<Mesh> _terrain_quad_mesh;
    };
}

#endif