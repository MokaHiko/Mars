#include "Input.h"

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

// Set keys to unpressed
bool mrs::Input::Keys[322] = {false};
bool mrs::Input::MouseButtons[4] = {false};

bool mrs::Input::IsKeyPressed(int key_code)
{
	if (key_code < 322) {
		return Keys[key_code];
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

