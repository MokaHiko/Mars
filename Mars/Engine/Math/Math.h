#ifndef MATH_H
#define MATH_H

#pragma once

#include <glm/glm.hpp>

namespace mrs {
    using Vector3 = glm::vec3;
    using Matrix4x4 = glm::mat4;

    bool DecomposeTransform(const Matrix4x4& transform, Vector3& translation, Vector3& rotation, Vector3& scale);
}
#endif