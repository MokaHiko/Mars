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
        // Particle Properties
        uint32_t max_particles = 256;
        uint32_t live_particles = 0;

        float life_time = 1.0f;
        float emission_rate = 10.0f;
        float particle_size = 1.0f;

        EmissionShape emission_shape = EmissionShape::Cone;
        std::shared_ptr<Material> texture;
    public:
        bool running = true;
        bool repeating = false;

        // Duration of particle system if not repeating
        float duration = 2.0f;

        // initial velocity multiplier
        glm::vec2 velocity{ 1.0f };
        glm::vec3 color{ 1.0f };

        // Time particle simulation has been running
        float time = 0.0f;

        void Play()
        {
            running = true;
            time = 0.0f;
        }
        void Stop()
        {
            running = false;
            live_particles = 0;
        }
        void Pause()
        {
            running = false;
            live_particles = 0;
        }

        // Sets reset flag to true
        void Reset() {
            reset = true;
        };
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

        // Pipeline handle to cache offset when destroyed
        ParticleRenderPipeline *pipeline;
    };
}

#endif