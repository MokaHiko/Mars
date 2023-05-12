#ifndef UTILS_H
#define UTILS_H

#pragma once

#include <cstdint>
#include <vector>
#include <functional>

namespace util {

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

	// Resizes buffer and returns raw file contents
	void read_file(const char* file_path, std::vector<char>& buffer);
}

#endif