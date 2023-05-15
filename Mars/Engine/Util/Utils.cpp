#include "Utils.h"

#include <fstream>

#include <string>
#include <SDL.h>

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

util::Timer::Timer(std::function<void(const Timer&)> finished_callback)
{
	then_ = SDL_GetTicks();
	callback_ = finished_callback;
}

util::Timer::~Timer()
{
	now_ = SDL_GetTicks();
	delta_ = now_ - then_;
	delta_ /= 1000;

	if(callback_ != nullptr)
		callback_(*this);
}