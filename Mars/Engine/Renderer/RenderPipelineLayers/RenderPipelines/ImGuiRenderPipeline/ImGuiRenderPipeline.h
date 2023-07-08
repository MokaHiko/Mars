#ifndef IMGUIRENDERPIPELINE_H
#define IMGUIRENDERPIPELINE_H

#pragma once

#include "Renderer/RenderPipelineLayers/IRenderPipeline.h"

namespace mrs
{
    class ImGuiRenderPipeline : public IRenderPipeline
    {
    public:
        ImGuiRenderPipeline() {};
        ~ImGuiRenderPipeline() {};

        virtual void Init() override;

        virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame) override;
        virtual void End(VkCommandBuffer cmd) override;

    private:
        void SetupImGuiStyle( bool bStyleDark_, float alpha_);
        SDL_Window *_window_handle = nullptr;
    };
}
#endif