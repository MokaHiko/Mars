#include "ParticleComponents.h"

bool mrs::ParticleSystem::operator==(const ParticleSystem &other) const
{
    if(other.max_particles != max_particles) {
        return false;
    }

    if(other.duration != duration) {
        return false;
    }

    if(other.emission_shape != emission_shape) {
        return false;
    }

    if(other.emission_rate != emission_rate) {
        return false;
    }

    return true;
}

size_t mrs::ParticleSystem::Hash() const
{
    size_t result = std::hash<size_t>()(max_particles);

    size_t hash = (uint32_t)duration | (uint32_t)emission_shape << 8 | (uint32_t)emission_rate << 16;

    // Xor shuffle
    result ^= hash;

    return result;
}