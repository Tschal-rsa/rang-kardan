#ifndef REVSURFACE_HPP
#define REVSURFACE_HPP

#include "object3d.hpp"
#include "curve.hpp"
#include <tuple>

class RevSurface : public Object3D {

    Curve *pCurve;

public:
    RevSurface(Curve *pCurve, Material* material) : pCurve(pCurve), Object3D(material, Vector3f(-pCurve->maxRadius, pCurve->ymin - 3, -pCurve->maxRadius), Vector3f(pCurve->maxRadius, pCurve->ymax + 3, pCurve->maxRadius)) {
        // Check flat.
        for (const auto &cp : pCurve->getControls()) {
            if (cp.z() != 0.0) {
                printf("Profile of revSurface must be flat on xy plane.\n");
                exit(0);
            }
        }
    }

    ~RevSurface() override {
        delete pCurve;
    }

    Vector3f getNormal(const Vector3f &n, float u, float v) {
        if (!material->hasNormal()) return n.normalized();
        Vector3f normal = material->getNormal(u, v);
        Vector3f tangent = Utils::generateVertical(n);
        Vector3f binormal = Vector3f::cross(normal, tangent).normalized();
        return tangent * normal.x() + binormal * normal.y() + n * normal.z();
    }

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        // (PA2 optional TODO): implement this for the ray-tracing routine using G-N iteration.
        Vector3f origin = r.getOrigin(), direction = r.getDirection();
        Vector3f tKonta = (konta - origin) / direction, tMakria = (makria - origin) / direction;
        float tEnter = Utils::max(Utils::min(tKonta, tMakria)), tExit = Utils::min(Utils::max(tKonta, tMakria));
        if (tEnter < tExit && tExit >= 0 && tEnter < h.getT()) {
            Vector3f point = r.pointAtParameter(tEnter);
            float theta = atan2(-point.z(), point.x()) + M_PI;
            float tau = (pCurve->ymax - point.y()) / (pCurve->ymax - pCurve->ymin);
            Vector3f normal;
            if (!methodNewton(r, tEnter, tau, theta, normal, point) || !isnormal(tEnter) || !isnormal(tau) || !isnormal(theta) || tEnter < tmin || tEnter >= h.getT() || tau < pCurve->lowerBound || tau > pCurve->upperBound) {
                return false;
            }
            float u = theta / (2 * M_PI);
            float v = (tau - pCurve->lowerBound) / (pCurve->upperBound - pCurve->lowerBound);
            h.set(tEnter, material, getNormal(normal, u, v), material->getColor(u, v));
            return true;
        } else {
            return false;
        }
    }

protected:
    void getRevSurfacePoint(float tau, float theta, Vector3f &point, Vector3f &parTau, Vector3f &parTheta) {
        Quat4f rot;
        rot.setAxisAngle(theta, Vector3f::UP);
        Matrix3f rotMat = Matrix3f::rotation(rot);
        /* rotation Matrix is:
        cos(theta)  0   sin(theta)
            0       1       0
        -sin(theta) 0   cos(theta)
        */
        CurvePoint curvePoint = pCurve->getCurvePoint(tau);
        point = rotMat * curvePoint.V;
        parTau = rotMat * curvePoint.T;
        parTheta = Vector3f(-curvePoint.V.x() * sin(theta), 0, -curvePoint.V.x() * cos(theta));
    }

    bool methodNewton(const Ray &ray, float &t, float &tau, float &theta, Vector3f &normal, Vector3f &point) {
        Vector3f parTau(0), parTheta(0);
        for (int epoch = 0; epoch < 20; ++epoch) {
            tau = tau >= pCurve->upperBound ? (pCurve->upperBound - __FLT_EPSILON__) : (tau <= pCurve->lowerBound ? (pCurve->lowerBound + __FLT_EPSILON__) : tau);
            theta = theta < 0 ? (theta + 2 * M_PI) : (theta >= 2 * M_PI ? fmod(theta, 2 * M_PI) : theta);
            getRevSurfacePoint(tau, theta, point, parTau, parTheta);
            normal = Vector3f::cross(parTau, parTheta);
            Vector3f F = ray.pointAtParameter(t) - point;
            if (F.squaredLength() < 1e-4) {
                return true;
            }
            float denominator = Vector3f::dot(ray.getDirection(), normal);
            Vector3f crossProduct = Vector3f::cross(parTheta, F);
            t -= Vector3f::dot(parTau, crossProduct) / denominator;
            tau -= Vector3f::dot(ray.getDirection(), crossProduct) / denominator;
            theta += Vector3f::dot(ray.getDirection(), Vector3f::cross(parTau, F)) / denominator;
        }
        return false;
    }

};

#endif //REVSURFACE_HPP
