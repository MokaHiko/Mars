#include "PlayerShipController.h"

#include <Core/Input.h>
#include <Math/Math.h>

void PlayerShipController::OnCreate()
{
  _ship = GetComponent<mrs::Transform>().parent;

  if (!_ship)
  {
    // TODO: Disable script
    MRS_INFO("Required ship as direct parent!");
  }
}

void PlayerShipController::OnUpdate(float dt) {
  mrs::Transform& transform = _ship.GetComponent<mrs::Transform>();
  mrs::RigidBody2D& rb = _ship.GetComponent<mrs::RigidBody2D>();

  static float ms = 50.0f;
  static float total_time = 0;
  static float rotation_duration = 1.0f;
  static float target_rotation = 45.0f;

  glm::vec2 velocity = { 0, 0 };

  if (mrs::Input::IsKeyPressed(SDLK_q)) {
  }

  if (mrs::Input::IsKeyPressed(SDLK_w))
  {
    velocity += glm::vec2{ 0, ms };
  }
  else if (mrs::Input::IsKeyPressed(SDLK_s))
  {
    velocity += glm::vec2{ 0, -ms };
  }

  if (mrs::Input::IsKeyPressed(SDLK_d))
  {
    velocity += glm::vec2{ ms, 0 };

    total_time += dt;
    transform.rotation.y = mrs::Lerp(transform.rotation.y, target_rotation, total_time / rotation_duration);
    if (target_rotation - transform.rotation.y < 0.01f)
    {
      total_time = 0;
    }
  }
  else if (mrs::Input::IsKeyPressed(SDLK_a)) 
  {
    velocity += glm::vec2{ -ms, 0 };

    total_time += dt;
    transform.rotation.y = mrs::Lerp(transform.rotation.y, -target_rotation, total_time / rotation_duration);

    if (-target_rotation - transform.rotation.y < 0.01f) {
      total_time = 0;
    }
  }

  rb.SetVelocity(velocity);
}
