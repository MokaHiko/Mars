#include "ResourceManager.h"
#include <fstream>

namespace mrs {
	std::vector<char> ResourceManager::LoadBinary(const std::string &path)
	{
		std::ifstream file(path.c_str(), std::ios::ate | std::ios::binary);

		std::vector<char> buffer;
		if(file.is_open())
		{
			size_t file_size = file.tellg();
			buffer.resize(file_size);

			file.seekg(0);
			file.read(buffer.data(), file_size);
			file.close();
		}

		return buffer;
	}

	ResourceManager::ResourceManager()
	{

	}

	ResourceManager::~ResourceManager()
	{

	}
}