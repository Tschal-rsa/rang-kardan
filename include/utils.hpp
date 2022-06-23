#ifndef UTILS_H
#define UTILS_H

#include <random>
#include <vecmath.h>

class Utils {
public:
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
    static inline float gammaCorrect(float x) { 
        return pow(x, 1 / 2.2);
    }
    static Vector3f gammaCorrect(Vector3f vec) {
        return Vector3f(gammaCorrect(vec.x()), gammaCorrect(vec.y()), gammaCorrect(vec.z()));
    }
};

#endif