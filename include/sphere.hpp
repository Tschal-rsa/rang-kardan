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
        Vector3f normal = (squaredLengthToCenter > squaredRadius ? 1 : -1) * (r.pointAtParameter(t) - center) / radius;
        h.set(t, material, normal);
        return true;
    }

protected:
    Vector3f center;
    float radius;
};


#endif
