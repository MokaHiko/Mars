#ifndef CAMERA_H
#define CAMERA_H

#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>	

namespace mrs {

	enum class CameraType {
		Unknown,
		Orthographic,
		Perspective
	};

	enum class Camera_Movement
    {
        FORWARD,
        BACKWARD,
        UP,
        DOWN,
        LEFT,
        RIGHT,
    };

	class Camera
	{
	public:
		Camera(CameraType type, uint32_t aspect_w, uint32_t aspect_h, const glm::vec3& pos = glm::vec3(.0f));
		~Camera();

		// Returns view projection matrix as reference
		inline const glm::mat4& GetViewProj() const { return _view_proj; }

		// Returns view projection as reference
		inline const glm::mat4& GetProj() const { return _projection; }

		inline const glm::vec3& GetFront() const { return _front; }
		inline const glm::vec3& GetRight() const { return _right; }
		inline const glm::vec3& GetUp() const { return _up; }

		// Returns reference to position
		inline glm::vec3& GetPosition() { return _position; }

		void UpdateViewProj();

		// Changes camera type
		void SetType(CameraType type);
	public:
        // Camera Settings
        float _yaw = -90.0f;
        float _pitch = 0.0f;

        float _speed = 2.5f;
        float _zoom = 45.0f;
        float _sensitivity = 0.1f;

    private:
        // Camera Relative Position Attributes
		glm::vec3 _front{ 0.0f };
		glm::vec3 _right{ 0.0f };
		glm::vec3 _up{ 0.0f };
        glm::vec3 _worldup = glm::vec3(0, 1, 0);
	private:
		CameraType _type;

		glm::mat4 _view = {};
		glm::mat4 _projection = {};
		glm::mat4 _view_proj = {};

		glm::vec3 _position = {};
	};
}

#endif