#include "Effect.h"
#include "Scripting/Process.h"

void Effect::OnStart() 
{
	auto& effect = GetComponent<EffectProperties>();

	auto delay_destroy = CreateRef<mrs::DelayProcess>(effect.duration, [&](){
		QueueDestroy();
	});

	StartProcess(delay_destroy);
}
