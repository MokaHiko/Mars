#ifndef PLATFORM_H
#define PLATFORM_H

#pragma once

namespace mrs
{
    enum class LogLevel : uint16_t
    {
        Unknown = 0,
        Info = 1,
        Success = 2,
        Trace = 3,
        Error = 4,
        Warn = 5,
    };

    class Platform
    {
    public:
        static void Log(const char *msg, LogLevel level);

    public:
        // Checks if system has enough hard disk space
        static bool CheckStorage(size_t required_size);

        // Checks if system has enough physical and virtual memory
        static bool CheckMemory(size_t physical_size, size_t virtual_size);
    };
}

#endif