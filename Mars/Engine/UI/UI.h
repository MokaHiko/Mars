#ifndef UI_H
#define UI_H

#pragma once
#include "Renderer/Textures/Sprite.h"
#include "Font.h"

namespace mrs
{
    enum class UIRenderMode
    {
        WorldSpace, // World Space uses the same units as the scene
        ScreenSpace // Sreen Space will always use vw and vh units
    };

    // Allignment

    // Text
    struct Text
    {
        std::string text;

        Vector4 color;
        Ref<Font> font;
    };
}

#endif