#ifndef VULKANTEXTURE_H
#define VULKANTEXTURE_H

#pragma once

#include "VulkanStructures.h"
#include "Core/Memory.h"

namespace mrs {
    class Texture
    {
    public:
        Texture();
        ~Texture();

        static Ref<Texture> LoadFromAsset(const std::string& path, const std::string& alias = "");

        static Ref<Texture> Get(const std::string& path);

        AllocatedImage _image = {};
        VkImageView _image_view = {};

        uint32_t _width, _height = 0;
        VkFormat _format = {};

        std::string _name;

        std::vector<char> pixel_data;
    };
}
#endif