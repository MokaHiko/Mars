#ifndef PARTICLERENDERPIPELINE_H
#define PARTICLERENDERPIPELINE_H

#pragma once

#include "Core/Memory.h"
#include <glm/glm.hpp>

#include "ParticleComponents.h"
#include "Renderer/RenderPipelineLayers/IRenderPipeline.h"
#include "Toolbox/RandomToolBox.h"

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
        // Color Gradient
        glm::vec4 color_1 = glm::vec4(1.0f);
        glm::vec4 color_2 = glm::vec4(1.0f);
        
        // Shape
        float scale = 1.0f;

        // Emission
        float life_time; // particles lifetiem in seconds;
        float dt; // delta time this frame in seconds
        float emission_rate;
        uint32_t live_particles;
        uint32_t reset; // set to 1 if reset

        // Buffer offsets
        uint32_t buffer_offset; // Offset into global particle buffer
        uint32_t buffer_index; // Offset into global particle buffer array
    };

    struct ParticleSystemPushConstant
    {
        uint32_t count;
        uint32_t material_index;
    };

    class ParticleRenderPipeline : public IRenderPipeline
    {
    public:
        ParticleRenderPipeline(uint32_t max_particles);
        ~ParticleRenderPipeline();

        virtual void Init() override;

        virtual void Compute(VkCommandBuffer cmd, uint32_t current_frame, float dt, RenderableBatch* compute_batch);
        virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch) override;
        virtual void End(VkCommandBuffer cmd) override;

        virtual void OnPreRenderPass(VkCommandBuffer cmd, RenderableBatch* batch) override;

        void RegisterParticleSystem(ParticleSystem &particle_system);
        void CacheParticleSystemType(ParticleSystem &particle_system);

        virtual void OnRenderableDestroyed(Entity e) override;
    public:
        // Particle system type
        struct ParticleSystemType
        {
            // Offset of template instance in buffer
            uint32_t template_instance_buffer_offset = 0;

            // Size a single instance of the particle system in buffer
            size_t buffer_size = 0;

            // Offsets of free members in buffer
            std::vector<uint32_t> free_buffer_offsets;
        };

        // Particle system hash
        struct ParticleSystemHash
        {
            size_t operator()(const ParticleSystem &k) const 
            {
                return k.Hash();
            }
        };

        // Particle system cache
        std::unordered_map<ParticleSystem, ParticleSystemType, ParticleSystemHash> _particle_system_type_cache;

        // Next free offset into particles buffer
        size_t _buffer_next_free_offset;
    private:
        // Mesh
        VkDescriptorSet _global_data_set = VK_NULL_HANDLE;
        VkDescriptorSetLayout _global_data_set_layout = VK_NULL_HANDLE;

        VkDescriptorSetLayout _object_set_layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> _object_sets = {};

        VkDescriptorSetLayout _dir_light_layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>  _dir_light_sets = {};
    private:
        virtual void UploadResources();

        size_t _padded_particle_size = 0;
        size_t _padded_particle_parameter_size = 0;
        size_t _particle_parameters_size = 0;
        size_t _particle_buffer_size = 0;
    private:
        void InitComputeDescriptors();
        void InitComputePipeline();
        void InitComputeSyncStructures();

        void UpdateComputeDescriptorSets(uint32_t current_frame, float dt, RenderableBatch* batch);
        void RecordComputeCommandBuffers(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch);

        // Fills an array with properties of a particle system
        void FillParticleArray(const ParticleSystem& particle_system, std::vector<Particle>& particles);

        // Particle Compute Pipeline
        std::vector<AllocatedBuffer> _particle_storage_buffers;
        std::vector<AllocatedBuffer> _particle_parameter_buffers;

        VkPipeline _compute_pipeline;
        VkPipelineLayout _compute_pipeline_layout;

        std::vector<VkDescriptorSet> _compute_descriptor_sets;
        VkDescriptorSetLayout _compute_descriptor_set_layout;

        // Renderer max and live particles
        uint32_t _max_particles = 0;

        std::vector<VkFence> _compute_in_flight_fences = {};
        std::vector<VkSemaphore> _compute_in_flight_semaphores = {};
    private:
        void InitGraphicsDescriptors();
        void InitGraphicsPipeline();

        // Particle Graphics Pipeline
        VkPipeline _graphics_pipeline;
        VkPipelineLayout _graphics_pipeline_layout;

        std::vector<VkDescriptorSet> _graphics_descriptor_sets;
        VkDescriptorSetLayout _graphics_descriptor_set_layout;

        tbx::PRNGenerator<float> _random_generator{0.0f, 1.0f};
        tbx::PRNGenerator<float> _random_generator_negative_to_one{-1.0f, 1.0f};
    };
}

#endif