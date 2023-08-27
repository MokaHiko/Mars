#ifndef LOG_H
#define LOG_H

#pragma once

#include "Platform/Platform.h"

#ifdef MRS_DEBUG
	#define MRS_ASSERT(value, ...) mrs::Platform::Assert(value, __VA_ARGS__);
#else
	#define MRS_ASSERT(value, ...) 
#endif

#define MRS_ERROR(msg, ...) mrs::Platform::Log(mrs::LogLevel::Error, msg, __VA_ARGS__)
#define MRS_INFO(msg, ...)  mrs::Platform::Log(mrs::LogLevel::Info, msg, __VA_ARGS__)
#define MRS_TRACE(msg, ...) mrs::Platform::Log(mrs::LogLevel::Trace, msg, __VA_ARGS__)

#endif