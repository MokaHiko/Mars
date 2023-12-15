#include "DefaultRenderPipelineLayer.h"

#include "Renderer/RenderPipelineLayers/RenderPipelines/MeshRenderPipeline.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/ParticleRenderPipeline/ParticleRenderPipeline.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/ImGuiRenderPipeline/ImGuiRenderPipeline.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/PostProcessingRenderPipeline/PostProcessingRenderPipeline.h"
#include "Renderer/RenderPipelineLayers/RenderPipelines/SpriteRenderPipeline/SpriteRenderPipeline.h"

mrs::DefaultRenderPipelineLayer::DefaultRenderPipelineLayer()
{
	_name = "DefaultRenderPipelineLayer";

	PushRenderPipeline(new ImGuiRenderPipeline());

	PushRenderPipeline(new PostProcessingRenderPipeline());
	PushRenderPipeline(new ParticleRenderPipeline(10000));
	PushRenderPipeline(new SpriteRenderPipeline());
	PushRenderPipeline(new MeshRenderPipeline());
}
