#ifndef PARTICLECOMPONENTS_H
#define PARTICLECOMPONENTS_H

#pragma once

#include "Renderer/Vulkan/VulkanMaterial.h"
#include <glm/glm.hpp>

namespace mrs
{
    enum class EmissionShape : uint16_t
    {
        None,
        Circle,
        Cone,
    };

    class ParticleRenderPipeline;
    struct ParticleSystem
    {
    public:
        ParticleSystem()
        {
            material = Material::Get("default_particle");
        };

        // Material
        Ref<Material> material; 

        // Color Gradient
        glm::vec4 color_1 = glm::vec4(1.0f, 1, 1, 1);
        glm::vec4 color_2 = glm::vec4(1.0f, 1, 1, 0.0f);

        // Shape
        EmissionShape emission_shape = EmissionShape::Cone;
        float particle_size = 0.15f;
        float spread_angle = 45.0f;

        // Emission properties
        uint32_t max_particles = 16;
        uint32_t live_particles = 0;
        float life_time = 2.0f;
        float emission_rate = 3.0f;
        glm::vec2 velocity{ 1.0f, 1.0f };

        // State
        bool running = true;
        bool repeating = true;
        float duration = 2.0f; // Duration of particle system if not repeating
        float time = 0.0f;  // Time particle simulation has been running

        bool world_space = false;

        void Play();

        void Stop();

        void Pause();

        void Reset();

    public:
        friend class ParticleRenderPipeline;

        // Equality operator
        bool operator==(const ParticleSystem &other) const;

        // Hash function
        size_t Hash() const;

        // Offset into global particle buffer
        uint32_t buffer_offset = 0;

        // Particles need to be registered with system
        bool registered = false;

        // Indicates whether or not particle system positins and velocities must be reset
        bool reset = false;

        // Indiactes whether or not particles to stop 
        bool stop = false;

        // Pipeline handle to cache offset when destroyed
        ParticleRenderPipeline *pipeline;
    };
}

#endif