#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <vecmath.h>
#include "object3d.hpp"
#include "constant.hpp"

// transforms a 3D point using a matrix, returning a 3D point
static Vector3f transformPoint(const Matrix4f &mat, const Vector3f &point) {
    return (mat * Vector4f(point, 1)).xyz();
}

// transform a 3D directino using a matrix, returning a direction
static Vector3f transformDirection(const Matrix4f &mat, const Vector3f &dir) {
    return (mat * Vector4f(dir, 0)).xyz();
}

// TODO: implement this class so that the intersect function first transforms the ray
class Transform : public Object3D {
public:
    Transform() {}

    Transform(const Matrix4f &m, Object3D *obj) : o(obj) {
        transform = m.inverse();
        Vector3f vertices[8] = {
            Vector3f(o->konta.x() , o->konta.y() , o->konta.z() ),
            Vector3f(o->konta.x() , o->konta.y() , o->makria.z()),
            Vector3f(o->konta.x() , o->makria.y(), o->konta.z() ),
            Vector3f(o->konta.x() , o->makria.y(), o->makria.z()),
            Vector3f(o->makria.x(), o->konta.y() , o->konta.z() ),
            Vector3f(o->makria.x(), o->konta.y() , o->makria.z()),
            Vector3f(o->makria.x(), o->makria.y(), o->konta.z() ),
            Vector3f(o->makria.x(), o->makria.y(), o->makria.z())
        };
        Vector3f konta(0), makria(0);
        for (int i = 0; i < 8; ++i) {
            Vector3f vertex = transformPoint(m, vertices[i]);
            konta = Utils::min(konta, vertex);
            makria = Utils::max(makria, vertex);
        }
        setBound(konta, makria);
    }

    ~Transform() {
    }

    virtual bool intersect(const Ray &r, Hit &h, float tmin) {
        Vector3f trSource = transformPoint(transform, r.getOrigin());
        Vector3f trDirection = transformDirection(transform, r.getDirection());
        Ray tr(trSource, trDirection);
        bool inter = o->intersect(tr, h, tmin);
        if (inter) {
            h.set(h.getT(), h.getMaterial(), transformDirection(transform.transposed(), h.getNormal()).normalized(), h.getColor());
        }
        return inter;
    }

protected:
    Object3D *o; //un-transformed object
    Matrix4f transform;
};

#endif //TRANSFORM_H
