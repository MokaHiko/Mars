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
    // RenderPipeline Stack
    class RenderPipelineStack
    {
    public:
	    RenderPipelineStack();
	    ~RenderPipelineStack();

        void PushRenderPipeline(IRenderPipeline* render_pipeline_layer);
        void PopRenderPipeline(IRenderPipeline* render_pipeline_layer);

        const std::vector<IRenderPipeline*>& PipelineStack() const {return _pipeline_stack;}

        std::vector<IRenderPipeline*>::iterator begin() { return _pipeline_stack.begin(); } 
        std::vector<IRenderPipeline*>::iterator end() { return _pipeline_stack.end(); }

        std::vector<IRenderPipeline*>::reverse_iterator rbegin() { return _pipeline_stack.rbegin(); } 
        std::vector<IRenderPipeline*>::reverse_iterator rend() { return _pipeline_stack.rend(); }
    private:
        std::vector<IRenderPipeline*> _pipeline_stack;
        
        uint32_t _insert_index = 0;
    };

    // Application layer that owns the application renderer and all graphics pipeline that render to scenes
    // RenderPipelineLayers contain a stack of render pipelines that can be added or removed 
    class IRenderPipelineLayer : public Layer
    {
    public:
        IRenderPipelineLayer()
        {
            _name = "IRenderPipelineLayer";
        };

        // Returns a reference of the renderer used by the render pipeline
        Ref<Renderer> GetRenderer() const {return _renderer;}

        const RenderPipelineStack& PipelineLayers() const {return _render_pipeline_layers;}

        virtual ~IRenderPipelineLayer(){};

        virtual void OnAttach() final;
        virtual void OnDetatch() final;

        virtual void OnEnable() final;

        virtual void OnUpdate(float dt) final;
        virtual void OnImGuiRender() final;

        /// <summary>
        ///  Sets the camera for the render pipeline
        /// </summary>
        void SetCamera(Camera* camera)
        {
            _renderer->SetCamera(camera);
        }

        void GetFrameBuffer()
        {
            _renderer->GetCurrentFrame();
        }

		void PushRenderPipeline(IRenderPipeline* pipeline);
		void PopRenderPipeline(IRenderPipeline* pipeline);
        
        void OnRenderableCreated(entt::basic_registry<entt::entity>&, entt::entity entity);
        void OnRenderableDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity);

        void OnMaterialsUpdate();
    private:
        Ref<Renderer> _renderer = nullptr;
		RenderPipelineStack _render_pipeline_layers;
    };
}

#endif