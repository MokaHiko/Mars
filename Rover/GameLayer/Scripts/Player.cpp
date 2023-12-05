#include "Player.h"
#include "Core/Input.h"

#include "Unit.h"
#include <Physics/Collider.h>

void Player::OnStart()
{
	_scene = mrs::Application::Instance().GetScene();
	mrs::Entity cam = FindEntityWithScript<mrs::CameraController>();
	if (cam)
	{
		_camera = cam.GetScript<mrs::CameraController>();
		MRS_INFO("Found %s", cam.GetComponent<mrs::Tag>().tag.c_str());
	}
	else
	{
		MRS_ERROR("No camera controller script found");
	}
}

void Player::OnUpdate(float dt)
{
	// if (mrs::Input::IsMouseButtonDown(SDL_BUTTON_LEFT))
	// {
	// 	glm::vec2 mouse_pos = mrs::Input::GetMousePosition();
	// 	mrs::Ray ray = _camera->ScreenPointToRay(mrs::Input::GetMousePosition());

	// 	mrs::Collision col = mrs::Physics::Raycast(_scene, ray);
	// 	if(col.entity)
	// 	{
	// 		Select(col.entity);
	// 	}
	// 	else
	// 	{
	// 		for (auto& e : _selected_units)
	// 		{
	// 			// TODO: Remove Select UI
	// 		}
	// 		_selected_units.clear();
	// 	}
	// }

	// if(mrs::Input::IsMouseButtonDown((SDL_BUTTON_RIGHT)))
	// {
	// 	glm::vec2 mouse_pos = mrs::Input::GetMousePosition();
	// 	mrs::Ray ray = _camera->ScreenPointToRay(mrs::Input::GetMousePosition());

	// 	mrs::Collision col = mrs::Physics::Raycast(_scene, ray);
	// 	std::vector<glm::vec2> positions = GenerateBoxPositions({col.collision_points.a}, _selected_units.size());

	// 	int ctr = 0;
	// 	for (auto& e : _selected_units)
	// 	{
	// 		Unit* unit = dynamic_cast<Unit*>(e.GetComponent<mrs::Script>().script);
	// 		if (unit)
	// 		{
	// 			unit->MoveTo(positions[ctr++]);
	// 		}
	// 	}
	// }
}

void Player::OnCollisionEnter2D(mrs::Entity other)
{
}

void Player::Select(mrs::Entity unit)
{
	for(auto& u : _selected_units)
	{
		if(u == unit)
		{
			return;
		}
	}

	if(unit.GetScript<Unit>() != nullptr)
	{
		// TODO: Activate Select UI
		_selected_units.push_back(unit);
	}
}

std::vector<glm::vec2> Player::GenerateBoxPositions(const glm::vec2& position, uint32_t n_positions)
{
	//// TODO: Do not assume each unit is the same radius
	//float unit_radius = 2.0f;

	//std::vector<glm::vec2> positions(n_positions);
	//int side_length = static_cast<int>(glm::ceil(glm::sqrt(n_positions)) * unit_radius);

	//int half_length = static_cast<int>(side_length / 2);
	//int left = position.x - half_length;
	//int right = position.x + half_length;
	//int bottom = position.y - half_length;
	//int top = position.y + half_length;

	//// Going from top left to bottom right
	//glm::vec2 next_position = glm::vec2(left, top);
	//for (int i = 0; i < n_positions; i++)
	//{
	//	next_position.x += unit_radius * 2;

	//	if (next_position.x > right)
	//	{
	//		next_position.x = left;
	//		next_position.y -= unit_radius * 2;
	//	}

	//	positions[i] = next_position;
	//}

	//return positions;

	return {};
}

void Player::OnCreate()
{

}
