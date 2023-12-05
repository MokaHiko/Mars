#include "Effect.h"
#include "Scripting/Process.h"

void Effect::OnStart() 
{
	Init();

	auto& props = GetComponent<EffectProperties>();

	if(props.fixed_time)
	{
		auto delay_destroy = CreateRef<mrs::FixedDelayProcess>(props.duration, [&](){
			QueueDestroy();
		});
		StartProcess(delay_destroy);
	}
	else
	{
		auto delay_destroy = CreateRef<mrs::DelayProcess>(props.duration, [&](){
			QueueDestroy();
		});
		StartProcess(delay_destroy);
	}
}
