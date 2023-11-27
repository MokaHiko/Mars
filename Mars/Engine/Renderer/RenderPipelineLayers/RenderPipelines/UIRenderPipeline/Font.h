#ifndef FONT_H
#define FONT_H

#pragma once

#include "Renderer/Textures/Sprite.h"
#include "Math/Math.h"

namespace mrs
{
    // For now fonts only support capital A-B and numbers sorted (ew!)
    class Font
    {
    public:
        // Create a font form n sprite atlas
        Font(Ref<Sprite> atlas);

        // TODO: Support Font Creation form .ttf
        // Font(const char* path);

        const Rect& GetUVS(const char c) const;
    private:
        Ref<Sprite> _atlas;
        std::unordered_map<int, Rect> _uvs;
    };
}

#endif