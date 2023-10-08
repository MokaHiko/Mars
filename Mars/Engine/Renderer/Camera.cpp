#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include "Core/Log.h"

mrs::Camera::Camera(CameraType type, uint32_t aspect_w, uint32_t aspect_h, const glm::vec3 &pos)
	:_type(type), _position(pos), _aspect_w(aspect_w), _aspect_h(aspect_h)
{
	MRS_TRACE("Camera created!");

	_view_proj = glm::mat4(1.0f);
	_view = glm::translate(glm::mat4(1.0f), _position);

	SetType(_type);

	_view_proj = _projection * _view;
}

mrs::Camera::~Camera() 
{
}

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
	float zoom = 5;
	switch (type)
	{
	case CameraType::Perspective:
		_projection = glm::perspective(glm::radians(70.0f), static_cast<float>(_aspect_w) / static_cast<float>(_aspect_h), _near, _far);
		_projection[1][1] *= -1; // Reconfigure y values as positive for vulkan
		break;
	case CameraType::Orthographic:
		_projection = glm::ortho(0.0f, static_cast<float>(_aspect_w), 0.0f, static_cast<float>(_aspect_h), _near, _far);
		_projection[1][1] *= -1; // Reconfigure y values as positive for vulkan
		break;
	default:
		break;
	}

	_type = type;
}
