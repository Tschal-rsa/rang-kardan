#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include "utils.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>
using namespace std;

// TODO: implement this class and add more fields as necessary,
class Triangle: public Object3D {

public:
	Triangle() = delete;

    // a b c are three vertex positions of the triangle
	Triangle( const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m) : Object3D(m), vertices{a, b, c}, edges{a - b, a - c}, konta(Utils::min(Utils::min(a, b), c)), makria(Utils::max(Utils::max(a, b), c)), hasTexture(false), hasNormal(false) {
		normal = Vector3f::cross(edges[0], edges[1]).normalized();
	}

	void setTextures(const Vector2f &a, const Vector2f &b, const Vector2f &c) {
		textures[0] = a;
		textures[1] = b;
		textures[2] = c;
		hasTexture = true;
	}

	void setNormals(const Vector3f &a, const Vector3f &b, const Vector3f &c) {
		normals[0] = a;
		normals[1] = b;
		normals[2] = c;
		hasNormal = true;
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
		Vector3f p(ray.pointAtParameter(t));
		Vector3f pa(vertices[0] - p), pb(vertices[1] - p), pc(vertices[2] - p);
		float s1 = Vector3f::cross(pb, pc).length();
		float s2 = Vector3f::cross(pc, pa).length();
		float s3 = Vector3f::cross(pa, pb).length();
		hit.set(t, material, hasNormal ? (s1 * normals[0] + s2 * normals[1] + s3 * normals[2]).normalized() : normal, material->getColor());
		return true;
	}
	Vector3f normal;
	Vector3f vertices[3];
	Vector3f konta, makria;
	Vector2f textures[3];
	Vector3f normals[3];
protected:
	Vector3f edges[2];
	bool hasTexture, hasNormal;
};

#endif //TRIANGLE_H
