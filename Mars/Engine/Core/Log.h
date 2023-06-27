#ifndef LOG_H
#define LOG_H

#pragma once

#include "Platform/Platform.h"

#define MRS_ERROR(x) Platform::Log(x, LogLevel::Error)
#define MRS_INFO(x) Platform::Log(x, LogLevel::Info)
#define MRS_TRACE(x) Platform::Log(x, LogLevel::Trace)

#endif