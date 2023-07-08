#ifndef PLAYERCAMERA_H
#define PLAYERCAMERA_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

#include "PlayerController.h"

class PlayerCamera : public mrs::ScriptableEntity
{
public:
		mrs::Transform* _transform;
		mrs::Camera* _camera;

		mrs::Entity _player;
		mrs::Transform* _player_transform;

		glm::vec3 _vector_distance_from_player = {0.0f, 0.0f, 30.0f};
		
        virtual void OnStart() 
		{
			_transform = &GetComponent<mrs::Transform>();
			_camera = &GetComponent<mrs::Camera>();

			_player = FindEntityWithScript<PlayerController>();
			_player_transform = &_player.GetComponent<mrs::Transform>();
		}

        virtual void OnUpdate(float dt) 
		{
			_transform->position = _player_transform->position + _vector_distance_from_player;
            _camera->GetPosition() = _transform->position;
		}

private:
};

#endif