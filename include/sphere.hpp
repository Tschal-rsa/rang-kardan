#ifndef SPHERE_H
#define SPHERE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement functions and add more fields as necessary

class SphereBase : public Object3D {
public:
    SphereBase(): center(0), radius(1) {}

    SphereBase(Material *material, const Vector3f &center, float radius): Object3D(material), center(center), radius(radius) {}

    ~SphereBase() override = default;

    Vector3f getNormal(const Vector3f &n, float u, float v) {
        if (!material->hasNormal()) return n;
        Vector3f normal = material->getNormal(u, v);
        Vector3f tangent = Utils::generateVertical(n);
        Vector3f binormal = Vector3f::cross(normal, tangent).normalized();
        return (tangent * normal.x() + binormal * normal.y() + n * normal.z()).normalized();
    }

    virtual Vector3f getCenter(float time) = 0;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        Vector3f tCenter = getCenter(r.getTime());
        float rayDirectionLength = r.getDirection().length();
        Vector3f rayToCenter = tCenter - r.getOrigin();
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
        Vector3f normal = (r.pointAtParameter(t) - tCenter).normalized();
        float u = atan2(normal.x(), normal.z()) / (2 * M_PI) + 0.5;
        float v = -asin(normal.y()) / M_PI + 0.5;
        h.set(t, material, getNormal(normal, u, v), material->getColor(u, v));
        return true;
    }

    Ray generateBeam(float time = 0) const override {
        float x = Utils::randomEngine(-1, 1), y = Utils::randomEngine(-1, 1);
        float r2 = Utils::square(x) + Utils::square(y);
        while (r2 >= 1) {
            x = Utils::randomEngine(-1, 1), y = Utils::randomEngine(-1, 1);
            r2 = Utils::square(x) + Utils::square(y);
        }
        Vector3f dir(2 * x * sqrt(1 - r2), 2 * y * sqrt(1 - r2), 1 - 2 * r2);
        return Ray(center + radius * dir, dir, time);
    }

protected:
    Vector3f center;
    float radius;
};

class Sphere: public SphereBase {
public:
    Sphere(const Vector3f &center, float radius, Material *material) : SphereBase(material, center, radius) {
        setBound(center - radius, center + radius);
    }

    Vector3f getCenter(float time) override {
        return center;
    }
};

class MotionSphere: public SphereBase {
public:
    MotionSphere(const Vector3f &origCenter, const Vector3f &center, float radius, Material *material, float tStart, float tEnd): SphereBase(material, center, radius), origCenter(origCenter), path(center - origCenter), tStart(tStart), tEnd(tEnd), interval(tEnd - tStart) {
        Vector3f konta = Utils::min(origCenter, center);
        Vector3f makria = Utils::max(origCenter, center);
        setBound(konta - radius, makria + radius);
    }
    Vector3f getCenter(float time) {
        return time >= tEnd ? center : (time <= tStart ? origCenter : (
            origCenter + (time - tStart) / interval * path
        ));
    }
protected:
    Vector3f origCenter, path;
    float tStart, tEnd, interval;
};


#endif
