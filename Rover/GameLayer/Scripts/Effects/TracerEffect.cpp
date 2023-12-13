#include "TracerEffect.h"
#include <Renderer/RenderPipelineLayers/RenderPipelines/TrailRenderPipeline/Trails.h>

#include <Core/Time.h>

void TracerEffect::Init() 
{
	duration = GetComponent<EffectProperties>().duration;

	auto& trails = AddComponent<mrs::TrailRenderer>();
	//trails.mesh->Vertices()[0].position = target.GetComponent<mrs::Transform>().position;
}

void TracerEffect::OnUpdate(float dt) 
{
	time_elapsed += mrs::Time::DeltaTime();
	// GetComponent<mrs::Transform>().position = mrs::Lerp(p1, p2, time_elapsed / duration);

	auto& dir  = glm::normalize(p2 - p1);
	float distance = glm::length(p2 - GetComponent<mrs::Transform>().position);
	if(distance <= 0.5f)
	{
		QueueDestroy();
		return;
	}

	GetComponent<mrs::Transform>().position += dir * 200.0f * dt;
}
