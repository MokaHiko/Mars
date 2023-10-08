#ifndef ALGORITHM_H
#define ALGORITHM_H

#pragma once

#include "Collider.h"

namespace mrs {
    namespace algo {
        CollisionPoints FindRaySphereCollision(const Ray* ray, const SphereCollider* col_b, const Transform* transform_b);
        CollisionPoints FindRayPlaneCollision(const Ray* ray, const PlaneCollider* col_b, const Transform* transform_b);
    }
};


#endif