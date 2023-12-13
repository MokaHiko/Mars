#include "IRenderPipelineLayer.h"
#include "Core/Application.h"

#include "Renderer/Vulkan/VulkanAssetManager.h"

namespace mrs
{
	RenderPipelineStack::RenderPipelineStack()
		: _insert_index(0) {}

	RenderPipelineStack::~RenderPipelineStack()
	{
		for (IRenderPipeline* pipeline : _pipeline_stack)
		{
			delete pipeline;
		}
	}

	void RenderPipelineStack::PushRenderPipeline(IRenderPipeline* render_pipeline_layer)
	{
		_pipeline_stack.emplace(_pipeline_stack.begin() + _insert_index, render_pipeline_layer);
		_insert_index++;
	}

	void RenderPipelineStack::PopRenderPipeline(IRenderPipeline* render_pipeline_layer)
	{
		auto it = std::find(_pipeline_stack.begin(), _pipeline_stack.end(), render_pipeline_layer);

		if (it != _pipeline_stack.end())
		{
			_pipeline_stack.erase(it);
			_insert_index--;
		}
	}

	IRenderPipeline* IRenderPipelineLayer::FindPipeline(const std::string& name)
	{
		auto it = std::find_if(_render_pipeline_layers.begin(), _render_pipeline_layers.end(), [&](const IRenderPipeline* pipeline)
			{
				return pipeline->Name() == name;
			});

		if (it != _render_pipeline_layers.end())
		{
			return *it;
		}

		return nullptr;
	}

	void IRenderPipelineLayer::OnAttach()
	{
		// Initialize rendererer systems
		RendererInfo renderer_info = {};
		renderer_info.window = Application::Instance().GetWindow();
		renderer_info.graphics_settings = {};
		renderer_info.max_objects = 10000;
		renderer_info.max_materials = 100;

		_renderer = std::make_shared<Renderer>(renderer_info);
		_renderer->Init();
		_name = "IRenderPipelineLayer";

		// Initialize runtime asset manager
		VulkanAssetManager::Instance().Init(_renderer.get());

		// Load built in resources
		Ref<Texture> default_texture = Texture::LoadFromAsset("Assets/Models/white.boop_png", "default");

		// Initialize render pipelines
		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			IRenderPipeline* pipeline = (*it);

			// Convenience handles
			pipeline->_window = Application::Instance().GetWindow().get();
			pipeline->_renderer = _renderer.get();
			pipeline->_device = &_renderer->Device();

			pipeline->_render_pass = _renderer->SwapchainRenderPass();

			pipeline->Init();
			_renderable_batches[pipeline] = {};
		}
	}

	void IRenderPipelineLayer::OnDetatch()
	{
		// Shutdown runtime asset manager
		VulkanAssetManager::Instance().Shutdown();

		_renderer->Shutdown();
	}

	void IRenderPipelineLayer::OnEnable()
	{
	}

	void IRenderPipelineLayer::OnUpdate(float dt)
	{
		uint32_t current_frame_index = _renderer->CurrentFrame();
		VkCommandBuffer cmd = _renderer->CurrentFrameData().command_buffer;

		Scene* scene = Application::Instance().GetScene();
		_renderer->UpdateGlobalDescriptors(scene, current_frame_index);
		BuildBatches(scene);

		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->Compute(_renderer->CurrentFrameData().compute_command_buffer, current_frame_index, dt, &_renderable_batches[*it]);
		}

		_renderer->Begin(scene);
		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->OnPreRenderPass(cmd, &_renderable_batches[*it]);
		}

		_renderer->MeshPassStart(cmd, _renderer->_offscreen_framebuffers[current_frame_index], _renderer->_offscreen_render_pass);
		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->Begin(cmd, current_frame_index, &_renderable_batches[*it]);
		}

		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->End(cmd);
		}
		_renderer->MeshPassEnd(cmd);

		// OffScreen Image Pipeline Barrier
		VkImageMemoryBarrier view_port_image_barrier = {};
		view_port_image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		view_port_image_barrier.srcAccessMask = 0;
		view_port_image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		view_port_image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		view_port_image_barrier.image = _renderer->_offscreen_images[current_frame_index].image;

		VkImageSubresourceRange range = {};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.levelCount = 1;
		range.baseMipLevel = 0;
		range.layerCount = 1;
		range.baseArrayLayer = 0;
		view_port_image_barrier.subresourceRange = range;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &view_port_image_barrier);

		_renderer->MainPassStart(cmd);
		for (auto it = _render_pipeline_layers.rbegin(); it != _render_pipeline_layers.rend(); it++)
		{
			(*it)->OnMainPassBegin(cmd);
		}

		for (Layer* layer : Application::Instance().GetLayers())
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
		ClearBatches();
	}

	void IRenderPipelineLayer::PushRenderPipeline(IRenderPipeline* pipeline)
	{
		_render_pipeline_layers.PushRenderPipeline(pipeline);
	}

	void IRenderPipelineLayer::PopRenderPipeline(IRenderPipeline* pipeline)
	{
		_render_pipeline_layers.PopRenderPipeline(pipeline);
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

	void IRenderPipelineLayer::BuildBatches(Scene* scene)
	{
		auto renderables = scene->Registry()->view<MeshRenderer>();

		for (auto entity : renderables)
		{
			Entity e(entity, scene);
			auto& renderable = e.GetComponent<MeshRenderer>();
			Ref<EffectTemplate> base_template = renderable.GetMaterial()->BaseTemplate();

			for (const ShaderEffect* effect : base_template->shader_effects)
			{
				_renderable_batches[effect->render_pipeline].entities.push_back(e);
			}
		}

		{
			// TODO: Replace with Renderable Base Component
			auto n_renderables = scene->Registry()->view<ParticleSystem>();
			for (auto entity : n_renderables)
			{
				Entity e(entity, scene);
				auto& renderable = e.GetComponent<ParticleSystem>();
				Ref<EffectTemplate> base_template = renderable.material->BaseTemplate();

				for (const ShaderEffect* effect : base_template->shader_effects)
				{
					_renderable_batches[effect->render_pipeline].entities.push_back(e);
				}
			}
		}

		{
			auto n_renderables = scene->Registry()->view<Renderable>();
			for (auto entity : n_renderables)
			{
				Entity e(entity, scene);
				auto& renderable = e.GetComponent<Renderable>();
				Ref<EffectTemplate> base_template = renderable.material->BaseTemplate();

				for (const ShaderEffect* effect : base_template->shader_effects)
				{
					_renderable_batches[effect->render_pipeline].entities.push_back(e);
				}
			}
		}
	}
	void IRenderPipelineLayer::ClearBatches()
	{
		for (auto it = _renderable_batches.begin(); it != _renderable_batches.end(); it++)
		{
			it->second.entities.clear();
		}
	}
}