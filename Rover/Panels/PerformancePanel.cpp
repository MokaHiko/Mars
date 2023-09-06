#include "PerformancePanel.h"

#include <Renderer/RenderPipelineLayers/IRenderPipelineLayer.h>
#include <imgui.h>
#include <Core/Time.h>

mrs::PerformancePanel::PerformancePanel(EditorLayer *editor_layer, const std::string &name,IRenderPipelineLayer *render_pipeline_layer)
	: IPanel(editor_layer, name), _render_pipeline_layer(render_pipeline_layer)
{
	_renderer = _render_pipeline_layer->GetRenderer();
	MRS_ASSERT(_renderer != nullptr, "No Renderer Found!");
}

mrs::PerformancePanel::~PerformancePanel() {}

void mrs::PerformancePanel::Draw() 
{
	ImGui::Begin("Performance");
	ImGui::Text("Pipeline Layers %d", _render_pipeline_layer->PipelineLayers().PipelineStack().size());
	if(ImGui::CollapsingHeader("Pipeline Layers"))
	{
		for(IRenderPipeline* pipeline : _render_pipeline_layer->PipelineLayers().PipelineStack())
		{
			ImGui::CollapsingHeader(pipeline->Name().c_str());
		}
	}
	
	ImGui::Text("Frame Time ms : %0.4f", Time::DeltaTime() * 1000.0f);
	ImGui::End();
}
