#include "InputLayer.h"
#include "Input.h"

#include "Application.h"

void mrs::InputLayer::OnAttach()
{
	Window *w = Application::GetInstance().GetWindow().get();

	Input::window_size_x = w->GetWidth();
	Input::window_size_y = w->GetHeight();
}

void mrs::InputLayer::OnEvent(Event &event)
{
	int32_t key = -1;

	switch (event._event.type)
	{
	case SDL_WINDOWEVENT_SHOWN:
		break;
	case SDL_KEYDOWN:
		Input::Keys[event._event.key.keysym.scancode] = true;
		break;
	case SDL_KEYUP:
		key = (int32_t)event._event.key.keysym.scancode;
		Input::Keys[event._event.key.keysym.scancode] = false;
		break;
	case SDL_MOUSEBUTTONDOWN:
		Input::MouseButtons[event._event.button.button] = true;
		break;
	case SDL_MOUSEBUTTONUP:
		Input::MouseButtons[event._event.button.button] = false;
		break;
	case SDL_MOUSEMOTION:
		Input::last_x = Input::x;
		Input::last_y = Input::y;

		Input::x = event._event.motion.x;
		Input::y = event._event.motion.y;

		Input::x_rel = event._event.motion.xrel;
		Input::y_rel = event._event.motion.yrel;
		break;
	}
}
