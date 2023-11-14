#ifndef MATH_H
#define MATH_H

#pragma once

#include <glm/glm.hpp>

namespace mrs {
    using Vector2 = glm::vec2;
    using Vector3 = glm::vec3;
    using Vector4 = glm::vec4;

    using Matrix4x4 = glm::mat4;

    bool DecomposeTransform(const Matrix4x4& transform, Vector3& translation, Vector3& rotation, Vector3& scale);

    template<typename T>
    float Cos(T value_radians)
    {
        return glm::cos(value_radians);
    }

    template<typename T>
    float Sin(T value_radians)
    {
        return glm::sin(value_radians);
    }
}
#endif