#ifndef PARTICLERENDERPIPELINE_H
#define PARTICLERENDERPIPELINE_H

#pragma once

#include <glm/glm.hpp>

#include "ParticleComponents.h"
#include "Renderer/RenderPipelineLayers/IRenderPipeline.h"

namespace mrs
{
    struct Particle
    {
        glm::vec2 position;
        glm::vec2 velocity;
        glm::vec4 color;

        float time_elapsed = 0.0f;

        static const VkVertexInputBindingDescription &GetBinding();
        static const std::vector<VkVertexInputAttributeDescription> GetAttributes();
    };

    struct ParticleParameters
    {
        //dela time in second
        float life_time = 0.0f;
        float dt = 0.0f;

        // emission rate in seconds 
        float emission_rate = 0.0f;

        // Particles alive
        uint32_t live_particles = 0;

        // Offset into global particle buffer
        uint32_t buffer_offset = 0;

        // Offset into global particle buffer array
        uint32_t buffer_index = 0;

        // Set to 1 if reset 
        uint32_t reset = 0;
    };

    struct ParticleSystemPushConstant
    {
        uint32_t count;
    };

    class ParticleRenderPipeline : public IRenderPipeline
    {
    public:
        ParticleRenderPipeline(uint32_t max_particles);
        ~ParticleRenderPipeline();

        virtual void Init() override;

        virtual void Compute(VkCommandBuffer cmd, uint32_t current_frame, float dt) override;
        virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame) override;
        virtual void End(VkCommandBuffer cmd) override;

        void RegisterParticleSystem(ParticleSystem &particle_system);
        void CacheParticleSystemType(ParticleSystem &particle_system);
    public:
        // Particle system type
        struct ParticleSystemType
        {
            // Offset of first instance in buffer
            uint32_t first_instance_buffer_offset = 0;

            // Size of single particle system in bufer
            size_t buffer_size = 0;

            // Offsets of free members in buffer
            std::vector<uint32_t> free_buffer_offsets;
        };

        // Particle system hash
        struct ParticleSystemHash
        {
            size_t operator()(const ParticleSystem &k) const {
                return k.Hash();
            }
        };

        // Particle system cache
        std::unordered_map<ParticleSystem, ParticleSystemType, ParticleSystemHash> _particle_system_type_cache;

        // Next free offset into particles buffer
        size_t _buffer_next_free_offset;
    private:
        virtual void UploadResources();

        size_t _particle_parameters_size = 0;
        size_t _particle_buffer_size = 0;
    private:
        void InitComputeDescriptors();
        void InitComputePipeline();

        // Particle Compute Pipeline
        std::vector<AllocatedBuffer> _particle_storage_buffers;
        std::vector<AllocatedBuffer> _particle_parameter_buffers;

        VkPipeline _compute_pipeline;
        VkPipelineLayout _compute_pipeline_layout;

        std::vector<VkDescriptorSet> _compute_descriptor_sets;
        VkDescriptorSetLayout _compute_descriptor_set_layout;

        // Renderer max and live particles
        uint32_t _max_particles = 0;
    private:
        void InitGraphicsDescriptors();
        void InitGraphicsPipeline();

        // Particle Graphics Pipeline
        VkPipeline _graphics_pipeline;
        VkPipelineLayout _graphics_pipeline_layout;

        std::vector<VkDescriptorSet> _graphics_descriptor_sets;
        VkDescriptorSetLayout _graphics_descriptor_set_layout;

        // Quad to render particles with
        std::shared_ptr<Mesh> _quad_mesh;
    };
}

#endif