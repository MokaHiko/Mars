#include "DefaultRenderPipelineLayer.h"

mrs::DefaultRenderPipelineLayer::DefaultRenderPipelineLayer()
{
	_name = "DefaultRenderPipelineLayer";

	PushRenderPipeline(new ImGuiRenderPipeline());
	PushRenderPipeline(new ParticleRenderPipeline(50000));

	PushRenderPipeline(new MeshRenderPipeline());
	PushRenderPipeline(new SpriteRenderPipeline());
	PushRenderPipeline(new PostProcessingRenderPipeline());
}
