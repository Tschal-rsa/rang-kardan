#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "ray.hpp"
#include "hit.hpp"
#include <iostream>

// TODO: Implement Shade function that computes Phong introduced in class.
class Material {
public:

    explicit Material(const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0) :
            diffuseColor(d_color), specularColor(s_color), shininess(s) {

    }

    virtual ~Material() = default;

    virtual Vector3f getDiffuseColor() const {
        return diffuseColor;
    }


    Vector3f Shade(const Ray &ray, const Hit &hit,
                   const Vector3f &dirToLight, const Vector3f &lightColor) {
        float dotProduct = Vector3f::dot(dirToLight, hit.getNormal());
        Vector3f diffuse = diffuseColor * std::max<float>(0, dotProduct);
        Vector3f specular = specularColor * pow(std::max<float>(0, Vector3f::dot(-ray.getDirection(), 2 * dotProduct * hit.getNormal() - dirToLight)), shininess);
        Vector3f shaded = lightColor * (diffuse + specular);
        return shaded;
    }

protected:
    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;
};


#endif // MATERIAL_H
