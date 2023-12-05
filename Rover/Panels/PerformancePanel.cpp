#include "PerformancePanel.h"

#include <Renderer/RenderPipelineLayers/IRenderPipelineLayer.h>
#include <imgui.h>
#include <Core/Time.h>

mrs::PerformancePanel::PerformancePanel(EditorLayer& editor_layer, const std::string& name, IRenderPipelineLayer* render_pipeline_layer)
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
	if (ImGui::CollapsingHeader("Pipeline Layers"))
	{
		for (IRenderPipeline* pipeline : _render_pipeline_layer->PipelineLayers().PipelineStack())
		{
			if (ImGui::CollapsingHeader(pipeline->Name().c_str()))
			{
				for (const auto& descriptor : pipeline->Effect()->descriptor_sets)
				{
					ImGui::Text("Set: %d", descriptor.set);

					ImGui::Indent();
					for (auto it = descriptor.bindings.begin(); it != descriptor.bindings.end(); it++)
					{
						ImGui::Text("Binding: %d", it->first);

						ImGui::Indent();
						auto key = (const uint32_t)(it->first);
						ImGui::Text("Name: %s", descriptor.BindingName(it->first).c_str());
						ImGui::Unindent();
					}
					ImGui::Unindent();
				}
			}
		}
	}

	ImGui::Text("Frame Time ms : %0.6f", Time::DeltaTime() * 1000.0f);
	ImGui::Text("Fixed Delta Time ms : %0.6f", Time::FixedDeltaTime() * 1000.0f);
	ImGui::End();
}
