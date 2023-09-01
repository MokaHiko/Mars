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
		renderer_info.max_objects = 10000;
		renderer_info.max_materials = 100;

		_renderer = std::make_shared<Renderer>(renderer_info);
		_renderer->Init();
		_name = "IRenderPipelineLayer";

		// Subscribe to RenderableObject component signals
		Scene* scene = Application::GetInstance().GetScene();
		scene->Registry()->on_construct<RenderableObject>().connect<&IRenderPipelineLayer::OnRenderableCreated>(this);
		scene->Registry()->on_destroy<RenderableObject>().connect<&IRenderPipelineLayer::OnRenderableDestroyed>(this);
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
		_renderer->MeshPassStart(cmd, _renderer->_offscreen_framebuffers[current_frame_index], _renderer->_offscreen_render_pass);
		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->Begin(cmd, current_frame_index);
		}

		// Ends a mesh pass
		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->End(cmd);
		}
		_renderer->MeshPassEnd(cmd);

		// Begin frame buffer render pass
		_renderer->MainPassStart(cmd);

		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->OnMainPassBegin(cmd);
		}

		for(Layer* layer : Application::GetInstance().GetLayers()) 
		{
			layer->OnImGuiRender();
		}

		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->OnMainPassEnd(cmd);
		}
		_renderer->MainPassEnd(cmd);

		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->OnPostMainPass(cmd);
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

	void IRenderPipelineLayer::OnRenderableCreated(entt::basic_registry<entt::entity>&, entt::entity entity)
	{
		static Scene* scene = Application::GetInstance().GetScene();
		Entity e{ entity, scene };

		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->OnRenderableCreated(e);
		}
	}

	void IRenderPipelineLayer::OnRenderableDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity)
	{
		static Scene* scene = Application::GetInstance().GetScene();
		Entity e{ entity, scene };

		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->OnRenderableDestroyed(e);
		}
	}

	void IRenderPipelineLayer::OnImGuiRender()
	{
	}
	void IRenderPipelineLayer::OnMaterialsUpdate()
	{
		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->OnMaterialsUpdate();
		}
	}
}