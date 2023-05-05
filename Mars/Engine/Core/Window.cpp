#include "Window.h"

namespace mrs {

	Window::Window(const std::string& window_title, int width, int height)
		:_width(width), _height(height)
	{
		SDL_Init(SDL_INIT_VIDEO);

		_window = SDL_CreateWindow(window_title.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width,
			height,
			SDL_WINDOW_VULKAN);
	}

	Window::~Window()
	{
		SDL_DestroyWindow(_window);
		SDL_Quit();
	}

	bool Window::Update()
	{
		while (SDL_PollEvent(&_event)) {
			if (_event.type == SDL_QUIT) {
				return false;
			}
			if (_event.type == SDL_KEYDOWN) {
			}
		}

		return true;
	}

	void* Window::GetNativeWindow()
	{
		return (void*)_window;
	}
}