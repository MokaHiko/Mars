#ifndef MATH_H
#define MATH_H

#pragma once

#include <glm/glm.hpp>

namespace mrs {
    using Vector2 = glm::vec2;
    using Vector3 = glm::vec3;
    using Vector4 = glm::vec4;

    using Vector2Int = glm::ivec2;
    using Vector3Int = glm::ivec3;
    using Vector4Int = glm::ivec4;

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

    float Lerp(float a, float b, float t);
    Vector2 Lerp(const Vector2& start, const Vector2& end, float percent);
    Vector2 Slerp(const Vector2& start, const Vector2& end, float percent, bool mirror = false);
}
#endif