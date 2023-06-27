#include "TimeToolBox.h"

#include <SDL.h>

tbx::Timer::Timer(std::function<void(const Timer&)> finished_callback)
{
	then_ = SDL_GetTicks();
	callback_ = finished_callback;
}

tbx::Timer::~Timer()
{
	now_ = SDL_GetTicks();
	delta_ = now_ - then_;
	delta_ /= 1000;

	if(callback_ != nullptr)
		callback_(*this);
}