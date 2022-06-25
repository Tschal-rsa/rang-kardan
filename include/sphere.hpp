#ifndef SPHERE_H
#define SPHERE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement functions and add more fields as necessary

class Sphere : public Object3D {
public:
    Sphere() {
        // unit ball at the center
        center = Vector3f::ZERO;
        radius = 1;
    }

    Sphere(const Vector3f &center, float radius, Material *material) : Object3D(material) {
        this->center = center;
        this->radius = radius;
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        float rayDirectionLength = r.getDirection().length();
        Vector3f rayToCenter = center - r.getOrigin();
        float squaredLengthToCenter = rayToCenter.squaredLength();
        float squaredRadius = radius * radius;
        // bool outer = rayToCenter.squaredLength() > squaredRadius;
        float disToFoot = Vector3f::dot(r.getDirection(), rayToCenter) / rayDirectionLength;
        if (squaredLengthToCenter >= squaredRadius && disToFoot <= 0) {
            return false;
        }
        float squaredDisToCenter = squaredLengthToCenter - disToFoot * disToFoot;
        if (squaredDisToCenter > squaredRadius) {
            return false;
        }
        float disToCross = sqrt(squaredRadius - squaredDisToCenter);
        float t = (squaredLengthToCenter > squaredRadius ? disToFoot - disToCross : disToFoot + disToCross) / rayDirectionLength;
        if (t > h.getT() || t < tmin) {
            return false;
        }
        Vector3f normal = (r.pointAtParameter(t) - center) / radius;
        if (material->hasTexture()) {
            float u = atan2(normal.x(), normal.z()) / (2 * M_PI) + 0.5;
            float v = -asin(normal.y()) / M_PI + 0.5;
            h.set(t, material, normal, material->getColor(u, v));
        } else {
            h.set(t, material, normal, material->getColor());
        }
        return true;
    }

    Ray generateBeam() const override {
        float x = Utils::randomEngine(-1, 1), y = Utils::randomEngine(-1, 1);
        float r2 = Utils::square(x) + Utils::square(y);
        while (r2 >= 1) {
            x = Utils::randomEngine(-1, 1), y = Utils::randomEngine(-1, 1);
            r2 = Utils::square(x) + Utils::square(y);
        }
        Vector3f dir(2 * x * sqrt(1 - r2), 2 * y * sqrt(1 - r2), 1 - 2 * r2);
        return Ray(center + radius * dir, dir);
    }

protected:
    Vector3f center;
    float radius;
};


#endif
