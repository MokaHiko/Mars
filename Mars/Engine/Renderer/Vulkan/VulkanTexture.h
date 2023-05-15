#ifndef VULKANTEXTURE_H
#define VULKANTEXTURE_H

#pragma once

#include "VulkanStructures.h"

namespace mrs {
    class Texture
    {
    public:
        Texture();
        ~Texture();

        static std::shared_ptr<Texture> LoadFromAsset(const std::string& path, const std::string& alias = "");

        static std::shared_ptr<Texture> Get(const std::string& path);

        AllocatedImage _image = {};
        VkImageView _image_view = {};

        uint32_t _width, _height = 0;
        VkFormat _format = {};

        std::vector<char> pixel_data;
    };
}
#endif