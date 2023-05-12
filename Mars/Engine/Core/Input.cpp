#include "Input.h"

#include <SDL2/SDL.h>

// Set keys to unpressed
bool mrs::Input::Keys[322] = {false};

bool mrs::Input::IsKeyPressed(int key_code)
{
	if (key_code <= 322) {
		return Keys[key_code];
	}

	return false;
}
