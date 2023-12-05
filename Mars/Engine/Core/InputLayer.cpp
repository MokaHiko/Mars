#include "InputLayer.h"
#include "Input.h"

#include "Application.h"

#include <imgui/imgui.h>

mrs::InputLayer::InputLayer()
	:Layer("InputLayer")
{
}

mrs::InputLayer::~InputLayer() {}

void mrs::InputLayer::OnAttach()
{
	Window* w = Application::Instance().GetWindow().get();

	Input::window_size_x = w->GetWidth();
	Input::window_size_y = w->GetHeight();
}

void mrs::InputLayer::OnEvent(Event& event)
{
	if(!IsEnabled())
	{
		return;
	}

	int32_t key = -1;
	switch (event._event.type)
	{
	case SDL_WINDOWEVENT_SHOWN:
		break;
	case SDL_KEYDOWN:
		Input::LastKeys[event._event.key.keysym.scancode] = Input::Keys[event._event.key.keysym.scancode];
		Input::Keys[event._event.key.keysym.scancode] = true;
		break;
	case SDL_KEYUP:
		Input::LastKeys[event._event.key.keysym.scancode] = Input::Keys[event._event.key.keysym.scancode];
		key = (int32_t)event._event.key.keysym.scancode;
		Input::Keys[event._event.key.keysym.scancode] = false;
		break;
	case SDL_MOUSEBUTTONDOWN:
		Input::LastMouseButtons[event._event.button.button] = Input::MouseButtons[event._event.button.button];
		Input::MouseButtons[event._event.button.button] = true;
		break;
	case SDL_MOUSEBUTTONUP:
		Input::LastMouseButtons[event._event.button.button] = Input::MouseButtons[event._event.button.button];
		Input::MouseButtons[event._event.button.button] = false;
		break;
	case SDL_MOUSEMOTION:
		Input::last_x = Input::x;
		Input::last_y = Input::y;

#ifdef MRS_RELEASE
		Input::x = event._event.motion.x;
		Input::y = event._event.motion.y;
#endif
		Input::x_rel = event._event.motion.xrel;
		Input::y_rel = event._event.motion.yrel;
		break;
	}
}

void mrs::InputLayer::OnEnable()
{
	MRS_INFO("[Input Layer] Enabled!");
}

void mrs::InputLayer::OnDisable() 
{
	MRS_INFO("[Input Layer] Disabled!");
	Input::Clear();
}

void mrs::InputLayer::OnImGuiRender()
{
}
