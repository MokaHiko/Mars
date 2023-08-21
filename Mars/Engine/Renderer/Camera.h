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
		inline const glm::mat4& GetViewProj() const { return _view_proj; }

		// Returns view matrix as reference
		inline const glm::mat4& GetView() const { return _view; }

		// Returns view projection as reference
		inline const glm::mat4& GetProj() const { return _projection; }

		inline const glm::vec3& GetFront() const { return _front; }
		inline const glm::vec3& GetRight() const { return _right; }
		inline const glm::vec3& GetUp() const { return _up; }
		inline const glm::vec3& GetWorldUp() const { return _worldup; }

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