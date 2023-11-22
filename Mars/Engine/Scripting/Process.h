#ifndef PROCESS_H
#define PROCESS_H

#pragma once

#include "Core/Memory.h"

namespace mrs {
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
        bool IsAlive() const { return _state == Running || _state == Paused; }
        bool IsDead() const { return _state == Succeeded || _state == Failed || _state == Aborted; }
        bool IsRemoved() const { return _state == Removed; }
        bool IsPaused() const { return _state == Paused; }
    private:
        void SetState(ProcessState new_state) { _state = new_state; }

        ProcessState _state = Uninitialized;
        Ref<Process> _child_process;
    };

    // Invokes callback after a set delay
    class DelayProcess : public Process
    {
    public:
        DelayProcess(float time, std::function<void(void)> callback = nullptr);

        virtual void OnUpdate(float dt) override;
    private:
        float _time = 0;
        std::function<void(void)> _callback = nullptr;
    };

    class FloatLerpProcess : public Process
    {
    public:
        // Duration and rate are in seconds
        FloatLerpProcess(float* value, float a, float b, float duration, float rate = 1)
            :_value(value), _a(a), _b(b), _duration(duration), _rate(rate)
        {
        };

        virtual void OnUpdate(float dt) override
        {
            *_value = (_a * (1 - (_time / _duration))) + (_b * (_time / _duration));
            _time += dt * _rate;

            if (*_value - _b <= 0.01f)
            {
                Succeed();
            }
        };
    private:
        float* _value = nullptr;
        float _a, _b;
        float _duration, _rate;

        float _time = 0;
    };


}

#endif