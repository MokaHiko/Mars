#include "PlayerShipController.h"

#include <Core/Input.h>
#include <Math/Math.h>

#include "ShipMovement.h"
#include "ShipCombat.h"

#include "GameCamera.h"

#include "Physics/Physics.h"

void PlayerShipController::OnStart()
{
  auto& parent = GetComponent<mrs::Transform>().parent;
  _ship = parent.GetScript<Ship>();

  if (!_ship)
  {
    MRS_INFO("Required ship as direct parent!");
  }

  _camera = FindEntityWithScript<GameCamera>().GetScript<GameCamera>();
}


void PlayerShipController::OnCreate() 
{
}

void PlayerShipController::OnUpdate(float dt)
{
  // Movement Controls
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
  }
  else if (mrs::Input::IsKeyPressed(SDLK_a))
  {
    input += mrs::Vector2Left;
  }

  _ship->Movement().MoveTowards(input);

  // Combat Controls
  if (mrs::Input::IsKeyDown(SDLK_SPACE))
  {
    _ship->Combat().SwitchCombatMode();
  }

  if(mrs::Input::IsMouseButtonPressed(SDL_BUTTON_LEFT))
  {
    _ship->Combat().ManualFire();
  }

  if (mrs::Input::IsKeyDown(SDLK_LSHIFT))
  {
    _ship->Movement().Boost(15.0f, 0.25f);
  }
  if(mrs::Input::IsMouseButtonDown(SDL_BUTTON_RIGHT))
  {
  }
}
