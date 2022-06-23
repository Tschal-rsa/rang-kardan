#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement Plane representing an infinite plane
// function: ax+by+cz=d
// choose your representation , add more fields and fill in the functions

class Plane : public Object3D {
public:
    Plane() {
        normal = Vector3f::UP;
        d = 0;
    }

    Plane(const Vector3f &normal, float d, Material *m) : Object3D(m) {
        this->normal = normal.normalized();
        this->d = d;
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        float dotProduct = Vector3f::dot(normal, r.getDirection());
        if (dotProduct == 0) {
            return false;
        }
        float t = (d - Vector3f::dot(normal, r.getOrigin())) / dotProduct;
        if (t <= 0 || t > h.getT() || t < tmin) {
            return false;
        }
        h.set(t, material, dotProduct < 0 ? normal : -normal, material->getColor());
        return true;
    }

protected:
    Vector3f normal;
    float d;
};

#endif //PLANE_H
		

