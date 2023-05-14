#ifndef IMGUILAYER_H
#define IMGUILAYER_H

#pragma once

#include "Core/Layer.h"

#include <imgui_impl_sdl2.h>
namespace mrs {
class ImGuiLayer : public Layer
{
public:
    virtual void OnAttach() override;
    virtual void OnUpdate(float dt) override;
private:
    SDL_Window* _window_handle;
};
}

#endif