#include "IRenderPipelineLayer.h"
#include "Core/Application.h"

namespace mrs
{
	RenderPipelineStack::RenderPipelineStack()
		: _insert_index(0) {}

	RenderPipelineStack::~RenderPipelineStack()
	{
		for (IRenderPipeline *pipeline : _pipeline_stack)
		{
			delete pipeline;
		}
	}

	void RenderPipelineStack::PushRenderPipeline(IRenderPipeline *render_pipeline_layer)
	{
		_pipeline_stack.emplace(_pipeline_stack.begin() + _insert_index, render_pipeline_layer);
		_insert_index++;
	}

	void RenderPipelineStack::PopRenderPipeline(IRenderPipeline *render_pipeline_layer)
	{
		auto it = std::find(_pipeline_stack.begin(), _pipeline_stack.end(), render_pipeline_layer);

		if (it != _pipeline_stack.end())
		{
			_pipeline_stack.erase(it);
			_insert_index--;
		}
	}

	void IRenderPipelineLayer::OnAttach()
	{
		// Initialize rendererer systems
		RendererInfo renderer_info = {};
		renderer_info.window = Application::GetInstance().GetWindow();
		renderer_info.graphics_settings = {};

		_renderer = std::make_shared<Renderer>(renderer_info);
		_renderer->Init();
		_name = "IRenderPipelineLayer";

		// Subscribe to application signals
		Application::GetInstance().GetScene()->_entity_destroyed += [&](Entity e){
			OnEntityDestroyed(e);
		};
	}

	void IRenderPipelineLayer::OnDetatch()
	{
		_renderer->Shutdown();
	}

	void IRenderPipelineLayer::OnEnable()
	{
		// Upload resources in resource manager
		_renderer->UploadResources();

		// Initialize render pipelines
		for(auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			IRenderPipeline* pipeline = (*it);

			// Convenience handles
			pipeline->_scene = Application::GetInstance().GetScene();
			pipeline->_window = Application::GetInstance().GetWindow().get();
			pipeline->_renderer = _renderer.get();
			pipeline->_device = &_renderer->GetDevice();

			pipeline->_render_pass = _renderer->GetSwapchainRenderPass();
			pipeline->_render_pass_format = _renderer->GetSwapchainImageFormat();

			pipeline->_global_descriptor_set_layout = _renderer->GetGlobalSetLayout();
			pipeline->_object_descriptor_set_layout = _renderer->GetGlobalObjectSetLayout();
			pipeline->_global_descriptor_set = _renderer->GetGlobalDescriptorSet();

			pipeline->_asset_manager = _renderer->GetAssetManager();
		
			pipeline->Init();
		}
	}

	void IRenderPipelineLayer::OnUpdate(float dt)
	{
		uint32_t current_frame_index = _renderer->GetCurrentFrame();
		VkCommandBuffer cmd = _renderer->GetCurrentFrameData().command_buffer;

		Scene* scene = Application::GetInstance().GetScene();

		// Compute Begin
		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->Compute(_renderer->GetCurrentFrameData().compute_command_buffer, current_frame_index, dt);
		}

		// Graphics Begin
		_renderer->Begin(scene);

		// Pre passes
		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->OnPreRenderPass(cmd);
		}

		// Begins a mesh pass
		_renderer->MainPassStart(cmd, _renderer->_offscreen_framebuffers[current_frame_index], _renderer->_offscreen_render_pass);
		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->Begin(cmd, current_frame_index);
		}

		// ImGui
		for(Layer* layer : Application::GetInstance().GetLayers()) 
		{
			layer->OnImGuiRender();
		}

		// Ends a mesh pass
		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->End(cmd);
		}
		_renderer->MainPassEnd(cmd);

		// Post pass
		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->OnPostRenderpass(cmd);
		}

		// Submit commands
		_renderer->End();
	}

	void IRenderPipelineLayer::PushRenderPipeline(IRenderPipeline *pipeline)
	{
		_render_pipeline_layers.PushRenderPipeline(pipeline);
	}

	void IRenderPipelineLayer::PopRenderPipeline(IRenderPipeline *pipeline)
	{
		_render_pipeline_layers.PopRenderPipeline(pipeline);
	}

	void IRenderPipelineLayer::OnImGuiRender()
	{
	}

	void IRenderPipelineLayer::OnEntityDestroyed(Entity e)
	{
		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->OnEntityDestroyed(e);
		}
	}
	void IRenderPipelineLayer::OnMaterialsUpdate()
	{
		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->OnMaterialsUpdate();
		}
	}
}