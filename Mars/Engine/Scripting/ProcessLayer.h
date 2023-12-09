#ifndef PROCESSLAYER_H
#define PROCESSLAYER_H

#pragma once

#include "Core/Memory.h"
#include "Core/Layer.h"

#include "Process.h"

namespace mrs
{
    // The Process Layer handles cooperative multitasking (processes/coroutines) used in scripts
    class ProcessLayer : public Layer
    {
    public:
        ProcessLayer();
        ~ProcessLayer();

        virtual void OnEnable() override;
        virtual void OnDisable() override;

        virtual void OnImGuiRender() override;
        virtual void OnUpdate(float dt) override;
        virtual void OnFixedUpdate(float fixed_dt) override;
    public:
        void AttachProcess(Ref<Process> process);
        void AbortAllProcesses(bool immediate);

        uint32_t ProcessCount() const { return static_cast<uint32_t>(_processes.size()); }
    private:
        std::list<Ref<Process>> _processes;
    };
}

#endif