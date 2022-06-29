#ifndef UTILS_H
#define UTILS_H

#include <random>
#include <vecmath.h>
#include <string>
#include <sstream>
#include <functional>

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
    static float triangularDistribution() {
        float r = randomEngine() * 2;
        return r < 1 ? (sqrt(r) - 1) : (1 - sqrt(2 - r));
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

    static float min(const Vector3f &f) {
        return (f.x() < f.y() && f.x() < f.z()) ? f.x() : (f.y() < f.z() ? f.y() : f.z());
    }
    static float max(const Vector3f &f) {
        return (f.x() > f.y() && f.x() > f.z()) ? f.x() : (f.y() > f.z() ? f.y() : f.z());
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
        // return vec.z() == 0 ? Vector3f::FORWARD : Vector3f(-vec.z() / vec.x(), 0, 1).normalized();
        Vector3f vertical(Vector3f::cross(Vector3f::UP, vec));
        if (vertical.squaredLength() < 1e-6) {
            vertical = Vector3f::cross(Vector3f::RIGHT, vec);
        }
        return vertical.normalized();
    }

    static Vector2f sampleUnitCircle() {
        float theta = randomEngine() * 2 * M_PI;
        float r = sqrt(randomEngine());
        return {
            r * cos(theta),
            r * sin(theta)
        };
    }

    static Vector3f sampleReflectedRay(Vector3f w) {
        // Sample two vectors (u, v) s.t. (u, v, norm) are vertical.
        // Vector3f u = Vector3f::cross(Vector3f(1, 0, 0), norm);
        // if (u.squaredLength() < 1e-6) u = Vector3f::cross(Vector3f(0, 1, 0), norm);
        // u.normalize();
        // Vector3f v = Vector3f::cross(norm, u);
        // v.normalize();
        // float theta = randomEngine(0, 2 * M_PI); // Sample an angle in the vertical plane.
        // float phi = asin(pow(randomEngine(), 1. / (s + 1))); // Sample an angle between norm and the plane.
        // return (norm * cos(phi) + (u * cos(theta) + v * sin(theta)) * sin(phi)).normalized();
        float r1 = 2 * M_PI * Utils::randomEngine(), r2 = Utils::randomEngine(), r2s = sqrt(r2);
        Vector3f u = Vector3f::cross((fabs(w.x()) > .1 ? Vector3f::UP : Vector3f::RIGHT), w).normalized(), v = Vector3f::cross(w, u);
        return (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalized();
    }

    static void stringSplit(string str, char split, vector<string> &res) {
        istringstream iss(str);
        string token;
        while (getline(iss, token, split)) {
            res.push_back(token);
        }
    }
};

#endif