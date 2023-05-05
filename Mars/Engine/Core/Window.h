#ifndef WINDOW_H
#define WINDOW_H

#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#undef main

#include <string>

namespace mrs {
	class Window 
	{
	public:
		Window(const std::string& window_title, int width, int height);
		~Window();

		bool Update();
		void* GetNativeWindow();

		inline uint32_t GetWidth() const { return _width; }
		inline uint32_t GetHeight() const {return _height;}
	private:
		uint32_t _width;
		uint32_t _height;

		SDL_Event _event = {};
		SDL_Window* _window = nullptr;
	};
}

#endif