#ifndef IRENDERPIPELINELAYER_H
#define IRENDERPIPELINELAYER_H

#pragma once

#include "Core/Layer.h"

#include "Renderer/Vulkan/VulkanInitializers.h"
#include "Renderer/Vulkan/VulkanUtils.h"

#include "Renderer/Renderer.h"
#include "ECS/Scene.h"

#include "IRenderPipeline.h"

namespace mrs
{
    // Manages render pipelines and their lifetimes
    class RenderPipelineStack
    {
    public:
	    RenderPipelineStack();
	    ~RenderPipelineStack();

        void PushRenderPipeline(IRenderPipeline* render_pipeline_layer);
        void PopRenderPipeline(IRenderPipeline* render_pipeline_layer);

        std::vector<IRenderPipeline*>::iterator begin() { return _pipeline_stack.begin(); } 
        std::vector<IRenderPipeline*>::iterator end() { return _pipeline_stack.end(); }

        std::vector<IRenderPipeline*>::reverse_iterator rbegin() { return _pipeline_stack.rbegin(); } 
        std::vector<IRenderPipeline*>::reverse_iterator rend() { return _pipeline_stack.rend(); }
    private:
        std::vector<IRenderPipeline*> _pipeline_stack;
        uint32_t _insert_index = 0;
    };

    // Application layer that owns the application renderer and all graphics pipeline that render to scenes
    class IRenderPipelineLayer : public Layer
    {
    public:
        IRenderPipelineLayer() {};
        virtual ~IRenderPipelineLayer(){};

        virtual void OnAttach() final;
        virtual void OnDetatch() final;

        virtual void OnUpdate(float dt) final;
        virtual void OnImGuiRender() final;

		void PushRenderPipeline(IRenderPipeline* pipeline);
		void PopRenderPipeline(IRenderPipeline* pipeline);

    private:
        std::shared_ptr<Renderer> _renderer = nullptr;
		RenderPipelineStack _render_pipeline_layers;
    };
}

#endif