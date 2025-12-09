#ifndef VECTOR3_HPP
#define VECTOR3_HPP

#include <cmath>
#include <iostream>

class Vector3 {
public:
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& v) const {
        return Vector3(x + v.x, y + v.y, z + v.z);
    }

    Vector3 operator-(const Vector3& v) const {
        return Vector3(x - v.x, y - v.y, z - v.z);
    }

    Vector3 operator*(float t) const {
        return Vector3(x * t, y * t, z * t);
    }

    Vector3 operator/(float t) const {
        return Vector3(x / t, y / t, z / t);
    }

    Vector3 operator*(const Vector3& v) const {
        return Vector3(x * v.x, y * v.y, z * v.z);
    }

    float dot(const Vector3& v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector3 normalize() const {
        float len = length();
        if (len > 0.0f)
            return *this / len;
        return *this;
    }

    Vector3 reflect(const Vector3& normal) const {
        return *this - normal * (2.0f * this->dot(normal));
    }

    friend std::ostream& operator<<(std::ostream& os, const Vector3& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }
    Vector3 cross(const Vector3& v) const {
        return Vector3(
                y * v.z - z * v.y,
                z * v.x - x * v.z,
                x * v.y - y * v.x
        );
    }
};

#endif