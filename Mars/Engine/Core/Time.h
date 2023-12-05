#ifndef TIME_H
#define TIME_H

#pragma once

namespace mrs
{
    class Time
    {
    public:
        // delta time in milliseconds
        static float DeltaTime() {return _dt;}

        // The interval in seconds at which physics, particles and other fixed frame rate updates are performed.
        static float FixedDeltaTime() {return _fixed_dt;}
    private:
        friend class Application;
        static void SetDeltaTime(const float dt)
        {
            _dt = dt;
        }
        static void SetFixedDeltaTime(const float fixed_dt)
        {
            _fixed_dt = fixed_dt;
        }

        static float _dt;
        static float _fixed_dt;
    };
}

#endif