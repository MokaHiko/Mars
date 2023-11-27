#include "Components.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Math/Math.h"

std::unordered_map <std::string, std::function<mrs::ScriptableEntity* ()>> mrs::Script::script_instantiation_bindings;

void mrs::Transform::AddChild(Entity e)
{
  MRS_ASSERT(children_count < MAX_CHILDREN, "Max child count reached!");

  // Remove from previous parent if exists
  Entity previous_parent = e.GetComponent<Transform>().parent;
  if (previous_parent)
  {
    previous_parent.GetComponent<Transform>().RemoveChild(e);
  }

  // Parent to self
  e.GetComponent<Transform>().parent = self;

  // Add to child list
  children[children_count++] = e;
}

void mrs::Transform::RemoveChild(Entity e)
{
  int insert_index = -1;
  for (uint32_t i = 0; i < children_count; i++)
  {
    if (children[i] == e)
    {
      insert_index = i;
      break;
    }
  }

  if (insert_index < 0)
  {
    MRS_ERROR("Child is not parented by object!");
    return;
  }

  int back_index = children_count - 1;
  if (back_index > -1)
  {
    Entity back = children[back_index];
    if (back != e)
    {
      children[insert_index] = back;
    }
  }
  children_count--;

  // Parent child back to root
  e.GetComponent<Transform>().parent = Application::Instance().GetScene()->Root();
}

void mrs::Transform::UpdateModelMatrix()
{
  // TODO: Add Dirty Check
  if (parent)
  {
    model_matrix = parent.GetComponent<Transform>().model_matrix * LocalModelMatrix();
  }
  else
  {
    model_matrix = LocalModelMatrix();
  }
}

glm::mat4 mrs::Transform::LocalModelMatrix() {
  glm::mat4 model{ 1.0f };
  model = glm::translate(model, position);

  static Vector3 Up = { 0,1, 0 };
  static Vector3 Down = { 0,-1, 0 };
  static Vector3 Right = { 1, 0, 0 };
  static Vector3 Left = { -1, 0, 0 };
  static Vector3 Forward = { 0, 0, 1 };
  static Vector3 Back = { 0, 0, -1 };

  glm::quat quat_rot = glm::quat(glm::vec3(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z)));
  model *= glm::toMat4(quat_rot);
  model = glm::scale(model, scale);

  forward = glm::rotate(quat_rot, Forward);
  back = -forward;

  up = glm::rotate(quat_rot, Up);
  down = -up;

  right = glm::rotate(quat_rot, Right);
  left = -right;

  // Euler angles rotation 
  // model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0, 0.0, 0.0));
  // model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0, 1.0, 0.0));
  // model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0, 0.0, 1.0));

  return model;
}
