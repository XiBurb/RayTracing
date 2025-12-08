#ifndef SCENE_HPP
#define SCENE_HPP

#include <vector>
#include "Sphere.hpp"
#include "Ray.hpp"

struct Light {
    Vector3 position;
    Vector3 color;
    float intensity;

    Light(const Vector3& pos, const Vector3& color = Vector3(1, 1, 1),
          float intensity = 1.0f)
            : position(pos), color(color), intensity(intensity) {}
};

struct HitRecord {
    float t;
    Vector3 point;
    Vector3 normal;
    Material material;
    bool hit;

    HitRecord() : t(-1.0f), hit(false) {}
};

class Scene {
public:
    std::vector<Sphere> spheres;
    std::vector<Light> lights;
    Vector3 backgroundColor;

    Scene() : backgroundColor(0.5f, 0.7f, 1.0f) {}

    void addSphere(const Sphere& sphere) {
        spheres.push_back(sphere);
    }

    void addLight(const Light& light) {
        lights.push_back(light);
    }

    HitRecord intersect(const Ray& ray, float tMin = 0.001f, float tMax = 1000.0f) const {
        HitRecord closestHit;
        closestHit.t = tMax;
        closestHit.hit = false;

        for (const auto& sphere : spheres) {
            float t = sphere.intersect(ray);
            if (t > tMin && t < closestHit.t) {
                closestHit.hit = true;
                closestHit.t = t;
                closestHit.point = ray.pointAt(t);
                closestHit.normal = sphere.getNormal(closestHit.point);
                closestHit.material = sphere.material;
            }
        }

        return closestHit;
    }

    bool isInShadow(const Vector3& point, const Vector3& lightPos) const {
        Vector3 lightDir = (lightPos - point).normalize();
        float lightDistance = (lightPos - point).length();

        Ray shadowRay(point, lightDir);
        HitRecord shadowHit = intersect(shadowRay, 0.001f, lightDistance);

        return shadowHit.hit;
    }
};

#endif