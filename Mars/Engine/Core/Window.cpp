#include "Window.h"

#include <Core/Input.h>
#include <imgui_impl_sdl2.h>

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

			int32_t key = (int32_t)_event.key.keysym.sym;
			switch (_event.type) {
			case SDL_QUIT:
				return false;
				break;
			case SDL_KEYDOWN:
				if (key < 322) { Input::Keys[_event.key.keysym.sym] = true; }
				break;
			case SDL_KEYUP:
				if (key < 322) { Input::Keys[_event.key.keysym.sym] = false; }
				break;
			default:
				break;
			}
			ImGui_ImplSDL2_ProcessEvent(&_event);
		}

		return true;
	}

	void* Window::GetNativeWindow()
	{
		return (void*)_window;
	}
}