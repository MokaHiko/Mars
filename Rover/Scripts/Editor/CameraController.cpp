#include "CameraController.h"

#include <Core/Input.h>
#include <Physics/Physics.h>

void mrs::CameraController::OnCreate()
{
	// Get handles
	_camera = &GetComponent<Camera>();
	_camera_transform = &GetComponent<Transform>();
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

	// Clip
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


