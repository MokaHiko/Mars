#include "CameraController.h"

void mrs::CameraController::OnStart()
{
	// Get handles
	_camera = &GetComponent<Camera>();
	_camera_transform = &GetComponent<Transform>();
}

void mrs::CameraController::OnUpdate(float dt)
{

	if (_camera->_active)
	{
		HandleKeyBoardInput(dt);
		HandleMouseInput(dt);
		_camera->UpdateViewProj();
	}
}

void mrs::CameraController::HandleKeyBoardInput(float dt)
{
	Transform& camera_transform = *_camera_transform;

	if (Input::IsKeyPressed(SDLK_f))
	{
		if (_focused)
		{
			camera_transform = _focused.GetComponent<Transform>();
			camera_transform.position.z -= _distance;
		}
	}

	if (Input::IsKeyPressed(SDLK_SPACE))
	{
		camera_transform.position += _camera->Up() * _movement_speed * dt;
	}

	else if (Input::IsKeyPressed(SDLK_q))
	{
		camera_transform.position -= _camera->Up() * _movement_speed * dt;
	}

	if (Input::IsKeyPressed(SDLK_w))
	{
		camera_transform.position += _camera->Front() * _movement_speed * dt;
	}
	else if (Input::IsKeyPressed(SDLK_s))
	{
		camera_transform.position -= _camera->Front() * _movement_speed * dt;
	}

	if (Input::IsKeyPressed(SDLK_d))
	{
		camera_transform.position += _camera->Right() * _movement_speed * dt;
	}
	else if (Input::IsKeyPressed(SDLK_a))
	{
		camera_transform.position -= _camera->Right() * _movement_speed * dt;
	}

	_camera->GetPosition() = _camera_transform->position;
}

void mrs::CameraController::HandleMouseInput(float dt)
{
	if (!Input::IsMouseButtonPressed(SDL_BUTTON_RIGHT))
	{
		return;
	}

	if (float x = Input::GetAxis('x'))
	{
		_camera->GetYaw() += (x * _mouse_sensitivity);
	}

	if (float y = Input::GetAxis('y'))
	{
		float pitch_change = _camera->GetPitch();
		pitch_change -= (y * _mouse_sensitivity);

		_camera->GetPitch() = pitch_change;

		if (pitch_change > 89.0f)
		{
			_camera->GetPitch() = 89.0f;
		}

		if (pitch_change < -89.0f)
		{
			_camera->GetPitch() = -89.0f;
		}
	}
}
