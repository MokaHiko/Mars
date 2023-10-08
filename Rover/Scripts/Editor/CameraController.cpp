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

const mrs::Ray mrs::CameraController::ScreenPointToRay(const glm::vec2& point) const
{
	Ray r = {};
	float x = point.x;
	float y = point.y;

	// Note: No need to flip y ndc because camera projection matrix already does that 
	glm::vec2 ndc = {};
	ndc.x = (x * 2) / Input::window_size_x - 1.0f;
	ndc.y = (y * 2) / Input::window_size_y - 1.0f;

	glm::vec4 clip_space = { ndc.x, ndc.y, -1, 1.0f };

	// Clip space to eye space
	glm::vec4 eye_space = glm::inverse(_camera->GetProj()) * clip_space;
	eye_space.z = -1;
	eye_space.w = 0;

	// Eye space to world space
	glm::vec4 world_space = glm::inverse(_camera->GetView()) * eye_space;
	glm::vec3 ray_dir = glm::normalize(glm::vec3{ world_space.x, world_space.y, world_space.z });

	r.origin = _camera->GetPosition();
	r.direction = ray_dir;
	return r;
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
