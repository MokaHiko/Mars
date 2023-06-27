#include "Input.h"

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

uint32_t mrs::Input::window_size_x = 0;
uint32_t mrs::Input::window_size_y = 0;

// Set keys to unpressed
bool mrs::Input::Keys[322] = {false};
bool mrs::Input::MouseButtons[4] = {false};

// Mouse
int mrs::Input::last_x = 0;
int mrs::Input::last_y = 0;

int mrs::Input::x = 0;
int mrs::Input::y = 0;

int mrs::Input::x_rel = 0;
int mrs::Input::y_rel = 0;

void mrs::Input::Reset()
{
	x_rel = 0;
	y_rel = 0;
}

bool mrs::Input::IsKeyPressed(int key_code)
{
	auto scan_code = SDL_GetScancodeFromKey(key_code);
	if (scan_code < 322) {
		return Keys[scan_code];
	}

	return false;
}

bool mrs::Input::IsMouseButtonPressed(int key_code)
{
	if (key_code < 4) {
		return MouseButtons[key_code];
	}

	return false;
}

float mrs::Input::GetAxis(const char axis)
{
	if(axis == 'x' || axis == 'X')
	{
		return x_rel;
	}

	if(axis == 'y' || axis == 'Y')
	{
		return y_rel;
	}

	throw std::runtime_error("Mouse Input axis undefined");
}

glm::vec2 mrs::Input::GetMousePosition()
{
	return { x,y };
}
