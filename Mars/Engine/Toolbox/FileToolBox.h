#ifndef FILETOOLBOX_H
#define FILETOOLBOX_H

#pragma once

namespace tbx
{
	// Resizes buffer and returns raw file contents
	void read_file(const char* file_path, std::vector<char>& buffer);
}

#endif