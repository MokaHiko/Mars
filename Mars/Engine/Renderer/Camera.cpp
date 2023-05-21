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
	glm::vec3 front{};
	front.x = cos(glm::radians(_yaw)) * glm::cos(glm::radians(_pitch));
	front.y = sin(glm::radians(_pitch));
	front.z = sin(glm::radians(_yaw)) * glm::cos(glm::radians(_pitch));
	_front = glm::normalize(front);

	_right = glm::normalize(glm::cross(_front, _worldup));
	_up = glm::normalize(glm::cross(_right, _front));

	_view = glm::lookAt(_position, _position + _front, _up);
	_view_proj = _projection * _view;
}

void mrs::Camera::SetType(CameraType type)
{
	_type = type;
}
