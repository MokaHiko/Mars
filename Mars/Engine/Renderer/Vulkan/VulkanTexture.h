#ifndef VULKANTEXTURE_H
#define VULKANTEXTURE_H

#pragma once

#include "VulkanStructures.h"
#include "Core/Memory.h"

namespace mrs {
    class Texture
    {
    public:
        enum class SamplerType
        {
            Linear, 
            Nearest
        };
    public:
        Texture();
        ~Texture();
        static Ref<Texture> LoadFromAsset(const std::string& path, const std::string& alias = "");
        static Ref<Texture> Get(const std::string& path);

        void SetSamplerType(SamplerType type);

        AllocatedImage _image = {};
        VkImageView _image_view = {};

        uint32_t _width, _height = 0;
        VkFormat _format = {};
        SamplerType _sampler_type = SamplerType::Linear;

        std::string _name;

        std::vector<char> pixel_data;
    };
}
#endif