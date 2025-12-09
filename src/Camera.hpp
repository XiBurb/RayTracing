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
        float halfHeight = tan(theta / 2.0f);
        float halfWidth = aspectRatio * halfHeight;

        this->position = position;

        // Направление взгляда: от камеры к цели
        Vector3 w = (lookAt - position).normalize();  // <- ИЗМЕНИТЬ НА lookAt - position

        // Вспомогательные векторы
        Vector3 up = Vector3(0, 1, 0);
        Vector3 u = up.cross(w).normalize();  // Правое направление
        Vector3 v = w.cross(u);               // Верхнее направление

        // Вычисляем углы viewport
        horizontal = u * (2.0f * halfWidth);
        vertical = v * (2.0f * halfHeight);

        // Нижний левый угол
        lowerLeftCorner = position - u * halfWidth - v * halfHeight + w;
    }

    Ray getRay(float u, float v) const {
        return Ray(position,
                   lowerLeftCorner + horizontal * u + vertical * v - position);
    }
};

#endif