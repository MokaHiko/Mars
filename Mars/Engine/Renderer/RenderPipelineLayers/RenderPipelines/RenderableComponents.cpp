#include "RenderableComponents.h"
#include "Core/Application.h"
#include "Renderer/RenderPipelineLayers/IRenderPipelineLayer.h"

namespace mrs
{
	void MeshRenderer::SetMaterial(Ref<Material> new_material)
	{
		static IRenderPipelineLayer* render_pipeline_layer = (IRenderPipelineLayer*)Application::Instance().FindLayer("IRenderPipelineLayer");
		material = new_material;
		if(render_pipeline_layer)
		{
			render_pipeline_layer->OnMaterialsUpdate();
		}
	}

	void SpriteAnimator::Play(int frame)
	{
		if(frame < 0)
		{
		}

		playing = true;
	}

	void SpriteAnimator::Stop()
	{
		playing = false;
	}

}