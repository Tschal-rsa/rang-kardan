#ifndef UTILS_H
#define UTILS_H

#include <random>
#include <vecmath.h>

using namespace std;

class Utils {
public:
    static float square(float x) {
        return x * x;
    }
    static float randomEngine() {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_real_distribution<float> u(0.0, 1.0);
        return u(rng);
    }
    static float randomEngine(float lo, float hi) {
        return lo + randomEngine() * (hi - lo);
    }
    static inline float clamp(float x) { 
        return x < 0 ? 0 : (x > 1 ? 1 : x);
    }
    static Vector3f clamp(Vector3f vec) {
        return Vector3f(clamp(vec.x()), clamp(vec.y()), clamp(vec.z()));
    }
    static inline float relu(float x) {
        return x > 0 ? x : 0;
    }
    static Vector3f relu(Vector3f vec) {
        return Vector3f(relu(vec.x()), relu(vec.y()), relu(vec.z()));
    }
    static Vector3f min(const Vector3f &a, const Vector3f &b) {
        return Vector3f(::min(a.x(), b.x()), ::min(a.y(), b.y()), ::min(a.z(), b.z()));
    }
    static Vector3f max(const Vector3f &a, const Vector3f &b) {
        return Vector3f(::max(a.x(), b.x()), ::max(a.y(), b.y()), ::max(a.z(), b.z()));
    }
    static inline float gammaCorrect(float x, float gamma = 0.5) { 
        return pow(x, gamma);
    }
    static inline Vector3f gammaCorrect(Vector3f vec, float gamma = 0.5) {
        return Vector3f(pow(vec.x(), gamma), pow(vec.y(), gamma), pow(vec.z(), gamma));
    }
    static Vector3f generateVertical(const Vector3f &vec) {
        return vec.z() == 0 ? Vector3f::FORWARD : Vector3f(-vec.z() / vec.x(), 0, 1).normalized();
    }
    static Vector3f sampleReflectedRay(Vector3f norm, float s = 1) {
        // Sample two vectors (u, v) s.t. (u, v, norm) are vertical.
        Vector3f u = Vector3f::cross(Vector3f(1, 0, 0), norm);
        if (u.squaredLength() < 1e-6) u = Vector3f::cross(Vector3f(0, 1, 0), norm);
        u.normalize();
        Vector3f v = Vector3f::cross(norm, u);
        v.normalize();
        float theta = randomEngine(0, 2 * M_PI); // Sample an angle in the vertical plane.
        float phi = asin(pow(randomEngine(), 1. / (s + 1))); // Sample an angle between norm and the plane.
        return (norm * cos(phi) + (u * cos(theta) + v * sin(theta)) * sin(phi)).normalized();
    }
};

#endif