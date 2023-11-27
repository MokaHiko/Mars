#ifndef SPRITE_H
#define SPRITE_H

#pragma once

#include "Renderer/Vulkan/VulkanTexture.h"
#include "Core/Memory.h"

namespace mrs {
    struct Rect
    {
        uint32_t x;
        uint32_t y;

        float width;
        float height;
    };

    class Sprite
    {
    public:
        Sprite();
        ~Sprite();

        const Ref<Texture> Atlas() const {return _atlas;}
    private:
        Ref<Texture> _atlas;
        Rect _rect;
    };
}

#endif