#include "PlatformWin32.h"

#include <direct.h>
#include <windows.h>

#include "Core/Log.h"

namespace mrs
{
	void Platform::Log(const char *msg, LogLevel level)
	{
		HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);

		WORD w_attribute = (int16_t)level;
		SetConsoleTextAttribute(h_console, w_attribute);

		printf("%s\n", msg);
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