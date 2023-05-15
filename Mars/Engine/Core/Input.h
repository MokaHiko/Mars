#ifndef INPUT_H
#define INPUT_H

#pragma once


namespace mrs {

	// Wrapper around SDL input 
	class Input
	{
	public:
		// SDL has 322 input events
		static bool Keys[322];
	public:
		static bool IsKeyPressed(int key_code);
	};
}

#endif