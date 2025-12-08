#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "Vector3.hpp"
#include "Ray.hpp"

struct Material {
    Vector3 color;
    float ambient;
    float diffuse;
    float specular;
    float shininess;

    Material() : color(1, 1, 1), ambient(0.1f), diffuse(0.7f),
                 specular(0.3f), shininess(32.0f) {}

    Material(const Vector3& color, float ambient = 0.1f,
             float diffuse = 0.7f, float specular = 0.3f,
             float shininess = 32.0f)
            : color(color), ambient(ambient), diffuse(diffuse),
              specular(specular), shininess(shininess) {}
};

class Sphere {
public:
    Vector3 center;
    float radius;
    Material material;

    Sphere() : center(0, 0, 0), radius(1.0f) {}

    Sphere(const Vector3& center, float radius, const Material& material)
            : center(center), radius(radius), material(material) {}

    float intersect(const Ray& ray) const {
        Vector3 oc = ray.origin - center;
        float a = ray.direction.dot(ray.direction);
        float b = 2.0f * oc.dot(ray.direction);
        float c = oc.dot(oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;

        if (discriminant < 0) {
            return -1.0f;
        }

        float t1 = (-b - std::sqrt(discriminant)) / (2.0f * a);
        float t2 = (-b + std::sqrt(discriminant)) / (2.0f * a);

        if (t1 > 0.001f) return t1;
        if (t2 > 0.001f) return t2;

        return -1.0f;
    }

    Vector3 getNormal(const Vector3& point) const {
        return (point - center).normalize();
    }
};

#endif