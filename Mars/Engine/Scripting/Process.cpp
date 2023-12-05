#include "Process.h"
#include "Core/Time.h"

mrs::Process::Process() {}

mrs::Process::~Process() {}

Ref<mrs::Process> mrs::Process::RemoveChild()
{
	if (_child_process)
	{
		Ref<Process> popped = _child_process;
		_child_process = nullptr;

		return popped;
	}

	return nullptr;
}

Ref<mrs::Process> mrs::Process::Child()
{
	return _child_process;
}

mrs::DelayProcess::DelayProcess(float time, std::function<void(void)> callback)
{
	_time = time;
	_callback = callback;
}

void mrs::DelayProcess::OnUpdate(float dt)
{
	_time -= dt;
	if (_time <= 0)
	{
		if (_callback)
		{
			_callback();
		}
		Succeed();
	}
};

mrs::FixedDelayProcess::FixedDelayProcess(float time, std::function<void(void)> callback)
{
	_time = time;
	_callback = callback;
}

void mrs::FixedDelayProcess::OnUpdate(float dt)
{
	_time -= Time::FixedDeltaTime();
	if (_time <= 0)
	{
		if (_callback)
		{
			_callback();
		}
		Succeed();
	}
};

