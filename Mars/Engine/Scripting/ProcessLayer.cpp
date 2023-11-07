#include "ProcessLayer.h"
#include <algorithm>

#include "Core/Log.h"

mrs::ProcessLayer::ProcessLayer()
	:Layer("ProcessLayer")
{
}

mrs::ProcessLayer::~ProcessLayer()
{
}

void mrs::ProcessLayer::OnEnable()
{
	// TODO: Reset processes
}

void mrs::ProcessLayer::OnDisable() {}

void mrs::ProcessLayer::OnImGuiRender() {}

void mrs::ProcessLayer::OnUpdate(float dt) {
	auto it = _processes.begin();

	uint8_t success_count = 0;
	uint8_t fail_count = 0;

	while (it != _processes.end()) {
		Ref<Process> process = (*it);

		std::list<Ref<Process>>::iterator this_it = it;
		it++;

		if (process->State() == Process::Uninitialized)
		{
			process->OnInit();
		}

		if (process->State() == Process::Running)
		{
			process->OnUpdate(dt);
		}

		if (process->IsDead()) {
			switch (process->State())
			{
				case Process::ProcessState::Succeeded:
				{
					process->OnSuccess();
					if (process->Child()) 
					{
						AttachProcess(process->Child());
					}
					else 
					{
						success_count++;
					}
				} break;
				case Process::ProcessState::Failed:
				{
					process->OnFail();
					fail_count++;
				} break;
				case Process::ProcessState::Aborted:
				{
					process->OnAbort();
					fail_count++;
				} break;
			}

			_processes.erase(this_it);
		}
	}
}

void mrs::ProcessLayer::AttachProcess(Ref<Process> process)
{
	MRS_ASSERT(process != nullptr, "Process is null");
	_processes.push_back(process);
}

void mrs::ProcessLayer::AbortAllProcesses(bool immediate)
{
	// TODO: Abort
}