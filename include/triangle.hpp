#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>
using namespace std;

// TODO: implement this class and add more fields as necessary,
class Triangle: public Object3D {

public:
	Triangle() = delete;

    // a b c are three vertex positions of the triangle
	Triangle( const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m) : Object3D(m), vertices{a, b, c}, edges{a - b, a - c} {
		normal = Vector3f::cross(edges[0], edges[1]).normalized();
	}

	bool intersect( const Ray& ray,  Hit& hit , float tmin) override {
        Vector3f s = vertices[0] - ray.getOrigin();
		float denominator = Matrix3f(ray.getDirection(), edges[0], edges[1]).determinant();
		if (denominator == 0) {
			return false;
		}
		float t = Matrix3f(s, edges[0], edges[1]).determinant() / denominator;
		float beta = Matrix3f(ray.getDirection(), s, edges[1]).determinant() / denominator;
		float gamma = Matrix3f(ray.getDirection(), edges[0], s).determinant() / denominator;
		if (t <= 0 || beta < 0 || beta > 1 || gamma < 0 || gamma > 1 || beta + gamma > 1 || t > hit.getT() || t < tmin) {
			return false;
		}
		hit.set(t, material, normal, material->getColor());
		return true;
	}
	Vector3f normal;
	Vector3f vertices[3];
protected:
	Vector3f edges[2];
};

#endif //TRIANGLE_H
