#ifndef DISK_H
#define DISK_H

#include "object3d.hpp"
#include "utils.hpp"
#include <vecmath.h>
#include <cmath>

class Disk: public Object3D {
public:
    Disk(): center(0), normal(-Vector3f::UP), radius(1), d(0) {}

    Disk(const Vector3f &center, const Vector3f &normal, float radius, Material *material): Object3D(material), center(center), normal(normal), radius(radius), d(Vector3f::dot(center, normal)) {}

    ~Disk() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        float dotProduct = Vector3f::dot(normal, r.getDirection());
        if (dotProduct == 0) {
            return false;
        }
        float t = (d - Vector3f::dot(normal, r.getOrigin())) / dotProduct;
        if (t <= 0 || t > h.getT() || t < tmin || (r.pointAtParameter(t) - center).squaredLength() > Utils::square(radius)) {
            return false;
        }
        h.set(t, material, dotProduct < 0 ? normal : -normal, material->getColor());
        return true;
    }
    Ray generateAverageRay() const override {
        Vector3f vertical(Utils::generateVertical(normal));
        float theta = Utils::randomEngine(0, 2 * M_PI);
        Vector3f rotated(vertical * cos(theta) + Vector3f::cross(normal, vertical * sin(theta)) + Vector3f::dot(normal, vertical) * normal * (1 - cos(theta)));
        return Ray(center + rotated * Utils::randomEngine(0, radius), Utils::sampleReflectedRay(normal));
    }
protected:
    Vector3f center;
    Vector3f normal;
    float radius;
    float d;
};

#endif