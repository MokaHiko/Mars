#include "PlatformWin32.h"

#include <direct.h>
#include <windows.h>

#include "Core/Log.h"
#include <stdarg.h>
#include <stdio.h>

namespace mrs
{
	void Platform::ConsoleWrite(const char *message, uint8_t color)
	{
		// Set color
		HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		static uint8_t levels[6] = { 64, 2, 1, 8, 5, 5}; // FATAL, INFO, DEBUG, TRACE, WARN, ERROR
		SetConsoleTextAttribute(console_handle, levels[color]);

		// Output to debug console
		OutputDebugStringA(message);

		// Output to application console
		uint64_t length = strlen(message);
		LPDWORD number_written = 0;
		WriteConsoleA(console_handle, message, (DWORD)length, number_written, 0);
	}

	void Platform::Log(LogLevel level, const char *msg, ...)
	{
		static const char* level_strings[6] =
		{
			"[Unknown]: ",
			"[INFO]: ",
			"[SUCCESS]: ",
			"[TRACE]: ",
			"[ERROR]: ",
			"[WARN]: ",
		};

		// Buffer for log entries
		const uint32_t msg_length = 32000;
		char out_message[msg_length];
		memset(out_message, 0, sizeof(out_message));

		va_list arg_ptr;
		va_start(arg_ptr, msg);
		vsnprintf(out_message, msg_length, msg, arg_ptr);
		va_end(arg_ptr);

		// Format with level_string log name
		char fout_message[msg_length];
		sprintf(fout_message, "%s%s\n", level_strings[(uint8_t)level], out_message);

		ConsoleWrite(fout_message, (uint8_t)level);
	}

	bool Platform::CheckStorage(size_t required_size)
	{
		int const drive = _getdrive();
		struct _diskfree_t disk_free = {};

		_getdiskfree(drive, &disk_free);
		unsigned __int64 const needed_clusters = required_size / (disk_free.sectors_per_cluster * disk_free.bytes_per_sector);

		if (disk_free.avail_clusters < needed_clusters)
		{
			MRS_ERROR("Check Storage Failure: Not enough physical storage.");
			return false;
		}

		return true;
	}

	bool Platform::CheckMemory(size_t physical_size, size_t virtual_size)
	{
		MEMORYSTATUSEX status;
		GlobalMemoryStatusEx(&status);
		if (status.ullTotalPhys < physical_size)
		{
			// you don’t have enough physical memory. Tell the player to go get a
			// real computer and give this one to his mother.
			MRS_ERROR("CheckMemory Failure : Not enough physical memory.");
			return false;
		}
		// Check for enough free memory.
		if (status.ullAvailVirtual < virtual_size)
		{
			// you don’t have enough virtual memory available.
			// Tell the player to shut down the copy of Visual Studio running in the
			// background, or whatever seems to be sucking the memory dry.
			MRS_ERROR("CheckMemory Failure : Not enough virtual memory.");
			return false;
		}

		char *buff = new char[virtual_size];
		if (buff)
		{
			delete[] buff;
		}
		else
		{
			// even though there is enough memory, it isn’t available in one
			// block, which can be critical for games that manage their own memory
			MRS_ERROR("CheckMemory Failure : Not enough contiguos memory.");
			return false;
		}
	}
}