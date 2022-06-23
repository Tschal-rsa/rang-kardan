#ifndef CURVE_HPP
#define CURVE_HPP

#include "object3d.hpp"
#include <vecmath.h>
#include <vector>
#include <utility>

#include <algorithm>

// TODO (PA2): Implement Bernstein class to compute spline basis function.
//       You may refer to the python-script for implementation.

// The CurvePoint object stores information about a point on a curve
// after it has been tesselated: the vertex (V) and the tangent (T)
// It is the responsiblility of functions that create these objects to fill in all the data.
struct CurvePoint {
    Vector3f V; // Vertex
    Vector3f T; // Tangent  (unit)
};

class Curve : public Object3D {
protected:
    std::vector<Vector3f> controls;
public:
    explicit Curve(std::vector<Vector3f> points) : controls(std::move(points)) {}

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        return false;
    }

    std::vector<Vector3f> &getControls() {
        return controls;
    }

    virtual void discretize(int resolution, std::vector<CurvePoint>& data) = 0;

};

class BezierCurve : public Curve {
public:
    explicit BezierCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() < 4 || points.size() % 3 != 1) {
            printf("Number of control points of BezierCurve must be 3n+1!\n");
            exit(0);
        }
    }

    void discretize(int resolution, std::vector<CurvePoint>& data) override {
        data.clear();
        // TODO (PA2): fill in data vector
        int n = controls.size() - 1;
        float interval = 1.0 / resolution, t = 0;
        std::vector<Vector3f> casteljau;
        while (t <= 1.0) {
            casteljau.assign(controls.begin(), controls.end());
            for (int k = 1; k < n; ++k) {
                for (int i = 0; i <= n - k; ++i) {
                    casteljau[i] = (1 - t) * casteljau[i] + t * casteljau[i + 1];
                }
            }
            data.push_back({
                (1 - t) * casteljau[0] + t * casteljau[1],
                (casteljau[1] - casteljau[0]).normalized()
            });
            t += interval;
        }
    }

protected:

};

class BsplineCurve : public Curve {
public:
    BsplineCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() < 4) {
            printf("Number of control points of BspineCurve must be more than 4!\n");
            exit(0);
        }
    }

    void discretize(int resolution, std::vector<CurvePoint>& data) override {
        data.clear();
        // TODO (PA2): fill in data vector
        int n = controls.size() - 1;
        float space = 1.0 / (n + 4), interval = space / resolution, t = 3 * space, milestone = 4 * space;
        std::vector<Vector3f> boor, kardia;
        float tau, upsilon;
        for (int j = 3; j <= n; ++j) {
            while (t < milestone) {
                boor.assign(controls.begin() + j - 3, controls.begin() + j + 1);
                kardia.assign(boor.begin() + 1, boor.end());
                for (int i = 0; i < 3; ++i) {
                    kardia[i] -= boor[i];
                }
                for (int r = 0; r < 2; ++r) {
                    for (int i = 3; i > r; --i) {
                        tau = ((n + 4) * t - i - j + 3) / (3 - r);
                        boor[i] = (1 - tau) * boor[i - 1] + tau * boor[i];
                    }
                    for (int i = 2; i > r; --i) {
                        upsilon = ((n + 4) * t - i - j + 2) / (2 - r);
                        kardia[i] = (1 - upsilon) * kardia[i - 1] + upsilon * kardia[i];
                    }
                }
                tau = (n + 4) * t - j;
                data.push_back({
                    (1 - tau) * boor[2] + tau * boor[3],
                    kardia[2].normalized()
                });
                t += interval;
            }
            milestone += space;
        }
    }

protected:

};

#endif // CURVE_HPP
