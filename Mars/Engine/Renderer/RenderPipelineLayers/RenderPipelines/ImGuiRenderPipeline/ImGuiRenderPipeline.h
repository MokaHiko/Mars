#ifndef IMGUIRENDERPIPELINE_H
#define IMGUIRENDERPIPELINE_H

#pragma once

#include "Renderer/RenderPipelineLayers/IRenderPipeline.h"

namespace mrs
{
    class ImGuiRenderPipeline : public IRenderPipeline
    {
    public:
        ImGuiRenderPipeline();
        ~ImGuiRenderPipeline();

        virtual void Init() override;

        // Called after mesh render pass
        virtual void OnMainPassBegin(VkCommandBuffer cmd) override;

        // Called at the end of swapchain frame buffer render pass
        virtual void OnMainPassEnd(VkCommandBuffer cmd) override;

        virtual void OnPostMainPass(VkCommandBuffer cmd) override;
    private:
        void SetupImGuiStyle( bool bStyleDark_, float alpha_);
        SDL_Window *_window_handle = nullptr;
    };
}
#endif