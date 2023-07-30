#ifndef TERRAINRENDERPIPELINE_H
#define TERRAINRENDERPIPELINE_H

#pragma once

#include <glm/glm.hpp>
#include "Renderer/Vulkan/VulkanUtils.h"
#include "Renderer/RenderPipelineLayers/IRenderPipeline.h"

namespace mrs
{
    class TerrainRenderPipeline : public IRenderPipeline
    {
    public:
        virtual void Init();

        virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame) ;

        virtual void End(VkCommandBuffer cmd) ;

		virtual void OnPreRenderPass(VkCommandBuffer cmd);

        virtual void OnPostRenderpass(VkCommandBuffer cmd) ;

        virtual void OnEntityDestroyed(Entity e) ;

        virtual void OnMaterialsUpdate();
    private:
        VkPipeline _terrain_render_pipeline;
        VkPipelineLayout _terrain_pipeline_layout;

        VkDescriptorSet _terrain_descriptor_set;
        VkDescriptorPool _terrain_descriptor_set_layout;

        AllocatedBuffer _terrain_descriptor_set_uniform_buffer;

        Texture _terrain_height_map;
        std::shared_ptr<Mesh> _quad_mesh;
    };
}

#endif