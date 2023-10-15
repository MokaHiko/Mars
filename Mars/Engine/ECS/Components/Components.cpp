#include "Components.h"
#include <glm/gtc/matrix_transform.hpp>

std::unordered_map <std::string, std::function<mrs::ScriptableEntity* ()>> mrs::Script::script_instantiation_bindings;

void mrs::Transform::AddChild(Entity e)
{
  MRS_ASSERT(children_count < MAX_CHILDREN, "Max child count reached!");

  children[children_count++] = e;
  e.GetComponent<Transform>().parent = self;
}

void mrs::Transform::UpdateModelMatrix()
{
  if (parent)
  {
    model_matrix = parent.GetComponent<Transform>().model_matrix * LocalModelMatrix();
  }
  else
  {
    model_matrix = LocalModelMatrix();
  }
}

glm::mat4 mrs::Transform::LocalModelMatrix() const {
  glm::mat4 model{ 1.0f };
  model = glm::translate(model, position);

  // TODO: switch to quaternions
  model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0, 0.0, 0.0));
  model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0, 1.0, 0.0));
  model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0, 0.0, 1.0));

  model = glm::scale(model, scale);

  return model;
}
