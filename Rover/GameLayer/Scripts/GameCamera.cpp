#include "GameCamera.h"

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

	_follow = FindEntityWithScript<Ship>();
}

void GameCamera::OnUpdate(float dt) 
{
	if(_follow)
	{
		mrs::Transform& follow_transform = _follow.GetComponent<mrs::Transform>();

		_camera_transform->position = follow_transform.position;
		_camera_transform->position.z = follow_transform.position.z + _distance;

		_camera->GetPosition() = _camera_transform->position;
	}
}

