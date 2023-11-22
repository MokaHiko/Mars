#include "GameCamera.h"

#include "PlayerShipController.h"
#include "Ship.h"


GameCamera::GameCamera()
{

}

GameCamera::~GameCamera()
{
}

void GameCamera::OnStart()
{
	// Get handles
	_camera = &GetComponent<mrs::Camera>();
	_camera_transform = &GetComponent<mrs::Transform>();

	_follow = FindEntityWithScript<PlayerShipController>().GetComponent<mrs::Transform>().parent;

	auto wait_process = CreateRef<mrs::DelayProcess>(1);
	auto zoom_process = CreateRef<mrs::FloatLerpProcess>(&_distance, _distance, _zoom_distance, _zoom_duration);
	wait_process->AttachChild(zoom_process);

	StartProcess(wait_process);
}

void GameCamera::OnUpdate(float dt)
{
	if (_follow.IsAlive())
	{
		mrs::Transform& follow_transform = _follow.GetComponent<mrs::Transform>();

		// _camera_transform->position = follow_transform.position;
		// _camera_transform->position.z = follow_transform.position.z + _distance;

		_camera_transform->position = follow_transform.position + (glm::vec3(0, 10, 0));
		_camera_transform->position.z = _distance;
	}

	_camera->GetPosition() = _camera_transform->position;
}

