#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include "utils.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>


class Camera {
public:
    Camera(const Vector3f &center, const Vector3f &direction, const Vector3f &up, int imgW, int imgH) {
        this->center = center;
        this->direction = direction.normalized();
        this->horizontal = Vector3f::cross(this->direction, up).normalized();
        this->up = Vector3f::cross(this->horizontal, this->direction);
        this->width = imgW;
        this->height = imgH;
    }

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(const Vector2f &point) = 0;
    virtual Ray generateAverageRay(const Vector2f &point) = 0;
    virtual Ray generateDistributedRay(const Vector2f &point) = 0;
    virtual ~Camera() = default;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

protected:
    // Extrinsic parameters
    Vector3f center;
    Vector3f direction;
    Vector3f up;
    Vector3f horizontal;
    // Intrinsic parameters
    int width;
    int height;
};

// TODO: Implement Perspective camera
// You can add new functions or variables whenever needed.
class PerspectiveCamera : public Camera {

public:
    PerspectiveCamera(const Vector3f &center, const Vector3f &direction,
            const Vector3f &up, int imgW, int imgH, float angle, float disToFocalPlane = 1, float apertureRadius = 0, float tStart = 0, float tEnd = 0) : Camera(center, direction, up, imgW, imgH), disToFocalPlane(disToFocalPlane), apertureRadius(apertureRadius), tStart(tStart), tEnd(tEnd) {
        // angle is in radian.
        // halfLengthX = tan(angleX / 2);
        // halfLengthY = tan(angleY / 2);
        // pixelX = halfLengthX / imgW * 2;
        // pixelY = halfLengthY / imgH * 2;
        pixelX = pixelY = tan(angle / 2) / imgH * 2;
        halfLengthX = imgW * pixelX / 2;
        halfLengthY = imgH * pixelY / 2;
        rotate = Matrix3f(horizontal, -this->up, this->direction);
    }

    Ray generateRay(const Vector2f &point) override {
        // add depth of field
        float sampleX = Utils::randomEngine(-1, 1) * apertureRadius, sampleY = Utils::randomEngine(-1, 1) * apertureRadius;
        return Ray(center + horizontal * sampleX - up * sampleY, rotate * Vector3f((point.x() * pixelX - halfLengthX) * disToFocalPlane - sampleX, (halfLengthY - point.y() * pixelY) * disToFocalPlane - sampleY, disToFocalPlane).normalized(), Utils::randomEngine(tStart, tEnd));
    }

    Ray generateAverageRay(const Vector2f &point) override {
        // Add a (-1, 1) random interrupt to the ray.
        return generateRay(Vector2f(point.x() + Utils::randomEngine(-1, 1), point.y() + Utils::randomEngine(-1, 1)));
    }

    Ray generateDistributedRay(const Vector2f &point) override {
        // Add a cubic-like distributed random interrupt to the ray.
        float dx = Utils::triangularDistribution(), dy = Utils::triangularDistribution();
        return generateRay(Vector2f(point.x() + dx, point.y() + dy));
    }

protected:
    float halfLengthX, halfLengthY, pixelX, pixelY;
    float disToFocalPlane, apertureRadius;
    Matrix3f rotate;
    float tStart, tEnd;
};

#endif //CAMERA_H
