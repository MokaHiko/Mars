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
        // Prevent pointers from being invalidated on delete
        static constexpr auto in_place_delete = true;

		Camera(CameraType type = CameraType::Perspective, uint32_t aspect_w = 1600, uint32_t aspect_h = 900, const glm::vec3& pos = glm::vec3(0.0f));
		~Camera();

		bool _active = true;
		uint32_t _aspect_w = 0;
		uint32_t _aspect_h = 0;

		float _near = 0.1f;
		float _far = 1000.0f;

		// Set active
		void SetActive(bool is_active) {_active = is_active;}

		// Returns whether or not camera is active
		const bool IsActive() const {return _active;}

		// Returns view projection matrix as reference
		const glm::mat4& GetViewProj() const { return _view_proj; }

		// Returns view matrix as reference
		const glm::mat4& GetView() const { return _view; }

		// Returns view projection as reference
		const glm::mat4& GetProj() const { return _projection; }

		glm::vec3& Front() { return _front; }
		glm::vec3& Right() { return _right; }
		glm::vec3& Up() { return _up; }
		glm::vec3& WorldUp() { return _worldup; }

		float& Yaw() { return _yaw; }
		float& Pitch() { return _pitch; }

		// Returns camera type
		CameraType GetType() const {return _type;}

		// Returns reference to position
		glm::vec3& GetPosition() { return _position; }

		// Update camera 
		void UpdateViewProj();

		// Changes camera type
		void SetType(CameraType type);

		// Returns reference to yaw
		float& GetYaw() {return _yaw;}

		// Returns reference to pitch
		float& GetPitch() {return _pitch;}
    private:
        // Camera Relative Position Attributes
		glm::vec3 _front{ 0.0f };
		glm::vec3 _right{ 0.0f };
		glm::vec3 _up{ 0.0f };
        glm::vec3 _worldup = glm::vec3(0, 1, 0);

        float _yaw = -90.0f;
        float _pitch = 0.0f;
	private:
		CameraType _type;

		glm::mat4 _view = {};
		glm::mat4 _projection = {};
		glm::mat4 _view_proj = {};

		glm::vec3 _position = {};
	};
}

#endif