#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

mrs::Camera::Camera(CameraType type, uint32_t aspect_w, uint32_t aspect_h, const glm::vec3& pos)
	:_type(type), _position(pos)
{
	_view_proj = glm::mat4(1.0f);

	// TODO: Move camera to its own class
	_view = glm::translate(glm::mat4(1.0f), _position);
	_projection = glm::perspective(glm::radians(70.0f),
		static_cast<float>(aspect_w) / static_cast<float>(aspect_h),
		0.1f,
		1000.0f);
	_projection[1][1] *= -1; // Reconfigure y values as positive for vulkan

	_view_proj = _projection * _view;
}

mrs::Camera::~Camera() {}

void mrs::Camera::UpdateViewProj()
{
	_view = glm::translate(glm::mat4(1.0f), _position);
	_view_proj = _projection * _view;
}
