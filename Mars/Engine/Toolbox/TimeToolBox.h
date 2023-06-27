#ifndef TIMETOOLBOX_H
#define TIMETOOLBOX_H

#pragma once

namespace tbx
{
    // A timer that takes in a function to be called when out object is out of scope
    class Timer
    {
    public:
        Timer(std::function<void(const Timer&)> finished_callback = nullptr);
        ~Timer();

        // Delta time in seconds
        double delta_ = 0;
    private:
        std::function<void(const Timer&)> callback_;
        unsigned int then_, now_;
    };
}

#endif