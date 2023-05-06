#ifndef UTILS_H
#define UTILS_H

#pragma once

#include <cstdint>
#include <vector>

namespace util {

	// Resizes buffer and returns raw file contents
	void read_file(const char* file_path, std::vector<char>& buffer);
}

#endif