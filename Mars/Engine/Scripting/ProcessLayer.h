#ifndef PROCESSLAYER_H
#define PROCESSLAYER_H

#pragma once

#include "Core/Memory.h"
#include "Core/Layer.h"

namespace mrs
{
    class Process
    {
    public:
        enum ProcessState
        {
            // Processes that are neither dead nor alive
            Uninitialized = 0, // created but not running
            Removed, // removed from the process list but not destroyed; this can
            // happen when a process that is already running is parented
            // to another process.

            // Living processes
            Running, // initialized and running
            Paused, // initialized but paused

            // Dead processes
            Succeeded, // completed successfully
            Failed, // failed to complete
            Aborted, // aborted; may not have started
        };

        Process();
        ~Process();
    public:
        virtual void OnInit(void) { _state = Running; }
        virtual void OnUpdate(float dt) = 0;

        // Process end callbacks
        virtual void OnSuccess() {}
        virtual void OnFail() {}
        virtual void OnAbort() {}
    public:
        // Functions for ending the process.
        inline void Succeed() { SetState(Succeeded); }
        inline void Fail() { SetState(Failed); }

        inline void Pause() { SetState(Paused); }
        inline void Play() { SetState(Running); }
        inline void AttachChild(Ref<Process> process) { _child_process = process; }

        Ref<Process> RemoveChild();
        Ref<Process> Child();
    public:
        // Accessors
        ProcessState State() const { return _state; }
        bool IsAlive() const const { return _state == Running || _state == Paused; }
        bool IsDead() const const { return _state == Succeeded || _state == Failed || _state == Aborted; }
        bool IsRemoved() const { return _state == Removed; }
        bool IsPaused() const { return _state == Paused; }
    private:
        void SetState(ProcessState new_state) { _state = new_state; }

        ProcessState _state = Uninitialized;
        Ref<Process> _child_process;
    };

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
    public:
        void AttachProcess(Ref<Process> process);
        void AbortAllProcesses(bool immediate);

        uint32_t ProcessCount() const { return static_cast<uint32_t>(_processes.size()); }
    private:
        std::list<Ref<Process>> _processes;
    };

    // Invokes callback after a set delay
    class DelayProcess : public Process
    {
    public:
        DelayProcess(float time, std::function<void(void)> callback);

        virtual void OnUpdate(float dt) override;
    private:
        float _time = 0;
        std::function<void(void)> _callback = nullptr;
    };
}

#endif