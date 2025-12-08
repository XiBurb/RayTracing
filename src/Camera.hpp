#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Vector3.hpp"
#include "Ray.hpp"

class Camera {
public:
    Vector3 position;
    Vector3 lowerLeftCorner;
    Vector3 horizontal;
    Vector3 vertical;

    Camera() {
        float aspectRatio = 16.0f / 9.0f;
        float viewportHeight = 2.0f;
        float viewportWidth = aspectRatio * viewportHeight;
        float focalLength = 1.0f;

        position = Vector3(0, 0, 0);
        horizontal = Vector3(viewportWidth, 0, 0);
        vertical = Vector3(0, viewportHeight, 0);
        lowerLeftCorner = position - horizontal / 2.0f - vertical / 2.0f -
                          Vector3(0, 0, focalLength);
    }

    Camera(const Vector3& position, const Vector3& lookAt,
           float vfov, float aspectRatio) {
        float theta = vfov * 3.14159265359f / 180.0f;
        float h = std::tan(theta / 2.0f);
        float viewportHeight = 2.0f * h;
        float viewportWidth = aspectRatio * viewportHeight;

        Vector3 w = (position - lookAt).normalize();
        Vector3 u = Vector3(0, 1, 0).normalize();
        Vector3 v = Vector3(
                u.y * w.z - u.z * w.y,
                u.z * w.x - u.x * w.z,
                u.x * w.y - u.y * w.x
        ).normalize();
        u = Vector3(
                v.y * w.z - v.z * w.y,
                v.z * w.x - v.x * w.z,
                v.x * w.y - v.y * w.x
        );

        this->position = position;
        horizontal = u * viewportWidth;
        vertical = v * viewportHeight;
        lowerLeftCorner = position - horizontal / 2.0f - vertical / 2.0f - w;
    }

    Ray getRay(float u, float v) const {
        return Ray(position,
                   lowerLeftCorner + horizontal * u + vertical * v - position);
    }
};

#endif