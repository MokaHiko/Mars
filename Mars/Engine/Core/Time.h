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
    private:
        friend class Application;
        static void SetDeltaTime(const float dt)
        {
            _dt = dt;
        }

        static float _dt;
    };
}

#endif