#include "Utils.h"

#include <fstream>

void util::read_file(const char* file_path, std::vector<char>& buffer)
{
	std::ifstream file(file_path, std::ios::ate | std::ios::binary);

	if (file.is_open())
	{
		size_t file_size = file.tellg();
		buffer.resize(file_size);

		file.seekg(0);
		file.read(buffer.data(), file_size);

		return;
	}

	printf("Failed to read file: ");
	printf("%s\n", file_path);
}
