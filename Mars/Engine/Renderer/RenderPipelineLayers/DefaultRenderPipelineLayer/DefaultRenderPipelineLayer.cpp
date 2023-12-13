#include "DefaultRenderPipelineLayer.h"

mrs::DefaultRenderPipelineLayer::DefaultRenderPipelineLayer()
{
	_name = "DefaultRenderPipelineLayer";

	PushRenderPipeline(new ImGuiRenderPipeline());

	PushRenderPipeline(new PostProcessingRenderPipeline());
	PushRenderPipeline(new ParticleRenderPipeline(10000));
	PushRenderPipeline(new SpriteRenderPipeline());
	PushRenderPipeline(new MeshRenderPipeline());
}
