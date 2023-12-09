#ifndef INPUT_H
#define INPUT_H

#pragma once

#include <glm/glm.hpp>

namespace mrs {
	class Input
	{
	public:
		static uint32_t window_size_x;
		static uint32_t window_size_y;

		static bool LastKeys[322];
		static bool Keys[322];
		static bool LastMouseButtons[4];
		static bool MouseButtons[4];

		static int last_x; // mouse position relative to window x 
		static int last_y; // mouse position relative to window y

		static int x; // mouse position relative to window x 
		static int y; // mouse position relative to window y

		static int x_rel; // relative mouse motion x 
		static int y_rel; // relative mouse motion y

		static void Input::Clear();
		static void Input::Reset();
	public:
		// Returns true if key(key_code) is pressed
		static bool IsKeyPressed(int key_code);

		// Returns true if key (key_code) is pressed on the frame it is called
		static bool IsKeyDown(int key_code);

		// Returns true if mouse button (key_code) is pressed
		static bool IsMouseButtonPressed(int key_code);

		// Returns true if mouse button (key_code) is pressed on the frame it is caleld
		static bool IsMouseButtonDown(int key_code);

		// The value will be in the range -1 to 1 for keyboard and joystick input devices.
		// If value is mouse it will be mouse delta
		static float GetAxis(const char axis);

		// Returns current mouse position
		static glm::vec2 GetMousePosition();
	};
}

#endif