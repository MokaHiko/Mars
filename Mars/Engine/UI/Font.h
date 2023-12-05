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
        static Ref<Font> LoadFromYaml(Ref<Texture> atlas, const std::string& yaml_meta_data, const std::string& alias = "");

        // TODO: Support Font Creation form .ttf
        // Font(const char* path);
        const Rectangle& GetUVS(const char c) const;
    private:
        Ref<Texture> _atlas;
        std::unordered_map<int, Rectangle> _letters;
    };
}

#endif