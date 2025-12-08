#ifndef RAY_HPP
#define RAY_HPP

#include "Vector3.hpp"

class Ray {
public:
    Vector3 origin;
    Vector3 direction;

    Ray() {}
    Ray(const Vector3& origin, const Vector3& direction)
            : origin(origin), direction(direction.normalize()) {}

    Vector3 pointAt(float t) const {
        return origin + direction * t;
    }
};

#endif