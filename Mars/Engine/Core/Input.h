#ifndef INPUT_H
#define INPUT_H

#pragma once

#include <glm/glm.hpp>

namespace mrs {

	// Wrapper around SDL input 
	class Input
	{
	public:
		// SDL has 322 input events
		static bool Keys[322];
		static bool MouseButtons[4];
		static int x_relative, y_relative;
	public:
		static bool IsKeyPressed(int key_code);
		static bool IsMouseButtonPressed(int key_code);

		// Returns the relative offset of mouse from last position
		inline const static glm::vec2 MouseOffset() { return {x_relative, y_relative }; }
	};
}

#endif