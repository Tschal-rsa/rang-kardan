#ifndef OBJECT3D_H
#define OBJECT3D_H

#include "ray.hpp"
#include "hit.hpp"
#include "material.hpp"
#include "utils.hpp"

// Base class for all 3d entities.
class Object3D {
public:
    Vector3f konta, makria;

    Object3D() : material(nullptr), konta(1e100), makria(-1e100), isBounded(false) {}

    virtual ~Object3D() = default;

    explicit Object3D(Material *material): material(material), konta(1e100), makria(-1e100), isBounded(false) {}

    explicit Object3D(Material *material, const Vector3f &konta, const Vector3f &makria): material(material), konta(konta), makria(makria), isBounded(true) {}

    Material* getMaterial() {
        return material;
    }

    bool bounded() {
        return isBounded;
    }

    void setBound(const Vector3f &konta, const Vector3f &makria) {
        this->konta = konta;
        this->makria = makria;
        isBounded = true;
    }

    Vector3f getCenter() {
        return (konta + makria) / 2;
    }

    // Intersect Ray with this object. If hit, store information in hit structure.
    virtual bool intersect(const Ray &r, Hit &h, float tmin) = 0;

    virtual Ray generateBeam(float time = 0) const {
        return Ray(Vector3f::ZERO, Vector3f::ZERO);
    }
protected:
    bool isBounded;

    Material *material;
};

#endif

