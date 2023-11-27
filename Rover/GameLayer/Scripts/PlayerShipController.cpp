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

  static float ms = 5.0f;
  static float turn_speed = 120.0f;
  static float total_time = 0;
  static float rotation_duration = 1.0f;
  static float target_rotation = 45.0f;

  glm::vec2 input = { 0, 0 };
  if (mrs::Input::IsKeyPressed(SDLK_w))
  {
    input += mrs::Vector2Up;
  }
  else if (mrs::Input::IsKeyPressed(SDLK_s))
  {
    input += mrs::Vector2Down;
  }

  if (mrs::Input::IsKeyPressed(SDLK_d))
  {
    input += mrs::Vector2Right;

    // total_time += dt;
    // transform.rotation.y = mrs::Lerp(transform.rotation.y, target_rotation, total_time / rotation_duration);
    // if (target_rotation - transform.rotation.y < 0.01f)
    // {
    //   total_time = 0;
    // }
  }
  else if (mrs::Input::IsKeyPressed(SDLK_a)) 
  {
    input += mrs::Vector2Left;
    // total_time += dt;
    // transform.rotation.y = mrs::Lerp(transform.rotation.y, -target_rotation, total_time / rotation_duration);

    // if (-target_rotation - transform.rotation.y < 0.01f)
    // {
    //   total_time = 0;
    // }
  }

  float w = -glm::radians(input.x * turn_speed);
  rb.SetAngularVelocity(w);

  mrs::Vector2 velocity =  transform.up * input.y * ms;
  rb.AddImpulse(velocity);
}
