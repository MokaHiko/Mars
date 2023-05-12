#ifndef CAMERA_H
#define CAMERA_H

#pragma once

#include <glm/glm.hpp>	

namespace mrs {

	enum class CameraType {
		Unknown,
		Orthographic,
		Perspective
	};

	class Camera
	{
	public:
		Camera(CameraType type, uint32_t aspect_w, uint32_t aspect_h, const glm::vec3& pos = glm::vec3(.0f));
		~Camera();

		// Returns view projection matrix as reference
		inline const glm::mat4& GetViewProj() const { return _view_proj; }

		// Returns reference to position
		inline glm::vec3& GetPosition() { return _position; }
		void UpdateViewProj();
	private:
		CameraType _type;

		glm::mat4 _view = {};
		glm::mat4 _projection = {};
		glm::mat4 _view_proj = {};

		glm::vec3 _position = {};
	};
}

#endif