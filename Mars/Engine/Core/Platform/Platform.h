#ifndef PLATFORM_H
#define PLATFORM_H

#pragma once

namespace mrs
{
    enum class LogLevel : uint8_t
    {
        Unknown = 0,
        Info,
        Success,
        Trace,
        Error,
        Warn
    };

    class Platform
    {
    public:
        static void Log(LogLevel level, const char* msg, ...);

        static void ConsoleWrite(const char* message, uint8_t color);
    public:
        // Checks if system has enough hard disk space
        static bool CheckStorage(size_t required_size);

        // Checks if system has enough physical and virtual memory
        static bool CheckMemory(size_t physical_size, size_t virtual_size);
    };
}

#endif