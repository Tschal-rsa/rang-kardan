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
    float ymin, ymax, maxRadius;
    float lowerBound, upperBound;
    explicit Curve(std::vector<Vector3f> points) : controls(std::move(points)), ymin(1e100), ymax(-1e100), maxRadius(0) {
        for (auto &point: controls) {
            ymin = std::min(ymin, point.y());
            ymax = std::max(ymax, point.y());
            maxRadius = std::max(maxRadius, std::max(fabs(point.x()), fabs(point.z())));
        }
    }

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        return false;
    }

    std::vector<Vector3f> &getControls() {
        return controls;
    }

    virtual void discretize(int resolution, std::vector<CurvePoint>& data) = 0;
    virtual CurvePoint getCurvePoint(float t) = 0;
};

class BezierCurve : public Curve {
public:
    explicit BezierCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() < 4 || points.size() % 3 != 1) {
            printf("Number of control points of BezierCurve must be 3n+1!\n");
            exit(0);
        }
        lowerBound = 0;
        upperBound = 1;
        n = controls.size() - 1;
        C = new float*[n + 1];
        C[0] = new float[n + 1];
        C[0][0] = 1;
        for (int j = 1; j <= n; ++j) {
            C[0][j] = 0;
        }
        for (int i = 1; i <= n; ++i) {
            C[i] = new float[n + 1];
            C[i][0] = 1;
            for (int j = 1; j <= n; ++j) {
                C[i][j] = C[i - 1][j - 1] + C[i - 1][j];
            }
        }
    }

    CurvePoint getCurvePoint(float t) override {
        Vector3f point(0), tangent(0);
        for (int i = 0; i <= n; ++i) {
            point += basis(i, n, t) * controls[i];
            tangent += dBasis(i, n, t) * controls[i];
        }
        return {point, tangent};
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

    ~BezierCurve() {
        for (int i = 0; i <= n; ++i) {
            delete[] C[i];
        }
        delete[] C;
    }

protected:
    float **C; // combinatorial numbers
    int n;
    float basis(int i, int n, float t) {
        return C[n][i] * pow(1 - t, n - i) * pow(t, i);
    }
    float dBasis(int i, int n, float t) {
        return n * (basis(i - 1, n - 1, t) - basis(i, n - 1, t));
    }
};

class BsplineCurve : public Curve {
public:
    BsplineCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() < 4) {
            printf("Number of control points of BspineCurve must be more than 4!\n");
            exit(0);
        }
        n = controls.size() - 1, k = 3;
        B = new float[n + k + 1];
        dB = new float[n + k + 1];
        lowerBound = knot(k);
        upperBound = knot(n + 1);
    }

    CurvePoint getCurvePoint(float t) override {
        for (int i = 0; i <= n + k; ++i) {
            B[i] = (knot(i) <= t && t < knot(i + 1));
        }
        for (int p = 1; p <= k; ++p) {
            for (int i = 0; i <= n + k - p; ++i) {
                if (p == k) {
                    dB[i] = p * (B[i] / (knot(i + p) - knot(i)) - B[i + 1] / (knot(i + p + 1) - knot(i + 1)));
                }
                B[i] = (t - knot(i)) / (knot(i + p) - knot(i)) * B[i] + (knot(i + p + 1) - t) / (knot(i + p + 1) - knot(i + 1)) * B[i + 1];
            }
        }
        Vector3f point(0), tangent(0);
        for (int i = 0; i <= n; ++i) {
            point += B[i] * controls[i];
            tangent += dB[i] * controls[i];
        }
        return {point, tangent};
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

    ~BsplineCurve() {
        delete[] B, dB;
    }

protected:
    float *B, *dB;
    int n, k;
    float knot(int i) {
        return float(i) / (n + k + 1);
    }
};

#endif // CURVE_HPP
