#include "TracerEffect.h"
#include <Renderer/RenderPipelineLayers/RenderPipelines/TrailRenderPipeline/Trails.h>

#include <Core/Time.h>

void TracerEffect::Init() 
{
	duration = GetComponent<EffectProperties>().duration;

	AddComponent<mrs::Renderable>().material = mrs::Material::Get("default_line");
	auto& trails = AddComponent<mrs::TrailRenderer>();
}

void TracerEffect::OnUpdate(float dt) 
{
	time_elapsed += mrs::Time::DeltaTime();
	// GetComponent<mrs::Transform>().position = mrs::Lerp(p1, p2, time_elapsed / duration);

	auto& dir  = glm::normalize(p2 - p1);

	float distance = glm::length(p2 - GetComponent<mrs::Transform>().position);
	if(distance <= 1.0f)
	{
		return;
	}

	GetComponent<mrs::Transform>().position += dir * 100.0f * dt;
}
